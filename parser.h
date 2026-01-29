#ifndef _PARSER_H_
#define _PARSER_H_

#include "lexer.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_PARAMS          32
#define MAX_LOCALS          256
#define MAX_STRUCT_MEMBERS  64
#define MAX_ENUM_VALUES     128
#define MAX_ARRAY_DIMS      8

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_PROCESS,
    NODE_SYSCALL,
    NODE_INTERRUPT,
    NODE_PARAM,
    NODE_BLOCK,
    NODE_VAR_DECL,
    NODE_ARRAY_DECL,
    NODE_STRUCT_DECL,
    NODE_UNION_DECL,
    NODE_ENUM_DECL,
    NODE_TYPEDEF,
    NODE_FIELD,
    NODE_EXPR_STMT,
    NODE_IF,
    NODE_WHILE,
    NODE_DO_WHILE,
    NODE_FOR,
    NODE_SWITCH,
    NODE_CASE,
    NODE_DEFAULT,
    NODE_RETURN,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_GOTO,
    NODE_LABEL,
    NODE_EMPTY,
    NODE_NUMBER,
    NODE_FLOAT,
    NODE_STRING,
    NODE_CHAR,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_ASSIGN,
    NODE_COMPOUND_ASSIGN,
    NODE_TERNARY,
    NODE_CALL,
    NODE_ARRAY_ACCESS,
    NODE_MEMBER_ACCESS,
    NODE_PTR_MEMBER,
    NODE_CAST,
    NODE_SIZEOF,
    NODE_ADDRESS_OF,
    NODE_DEREFERENCE,
    NODE_PRE_INC,
    NODE_PRE_DEC,
    NODE_POST_INC,
    NODE_POST_DEC,
    NODE_COMMA,
    NODE_INIT_LIST,
    NODE_CREATE,
    NODE_RESUME,
    NODE_SUSPEND,
    NODE_KILL,
    NODE_SLEEP,
    NODE_YIELD,
    NODE_WAIT,
    NODE_SIGNAL,
    NODE_GETPID,
    NODE_SEMAPHORE,
    NODE_TYPE,
    NODE_POINTER_TYPE,
    NODE_ARRAY_TYPE,
    NODE_FUNC_TYPE
} ast_node_type_t;

/* Data types */
typedef enum {
    TYPE_VOID,
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_ENUM,
    TYPE_FUNCTION,
    TYPE_PROCESS,           /* Xinu process type */
    TYPE_SEMAPHORE,         /* Xinu semaphore type */
    TYPE_PID,               /* Process ID type */
    TYPE_UNKNOWN
} data_type_t;

typedef enum {
    QUAL_NONE       = 0,
    QUAL_CONST      = 1 << 0,
    QUAL_VOLATILE   = 1 << 1,
    QUAL_UNSIGNED   = 1 << 2,
    QUAL_SIGNED     = 1 << 3,
    QUAL_STATIC     = 1 << 4,
    QUAL_EXTERN     = 1 << 5,
    QUAL_REGISTER   = 1 << 6
} type_qualifier_t;

struct ast_node;
struct type_info;
struct symbol;

typedef struct type_info {
    data_type_t base_type;
    uint32_t qualifiers;
    int pointer_depth;
    int array_dims;
    int array_sizes[MAX_ARRAY_DIMS];
    char struct_name[MAX_TOKEN_LEN];
    struct type_info *pointed_type;
    struct type_info *return_type;
    int num_params;
    struct type_info **param_types;
} type_info_t;

typedef struct ast_node {
    ast_node_type_t type;
    token_t token;
    type_info_t *data_type;
    
    union {
        int64_t int_val;
        double float_val;
        char char_val;
        char *string_val;
    } value;
    
    char name[MAX_TOKEN_LEN];
    char op[8];
    
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *extra;
    
    struct ast_node **children;
    int child_count;
    int child_capacity;
    
    int line;
    int column;
    const char *filename;
    
    struct symbol *symbol;
    bool is_lvalue;
    bool is_constant;
} ast_node_t;

typedef struct {
    lexer_t *lexer;
    token_t current;
    token_t previous;
    bool had_error;
    bool panic_mode;
    char error_msg[512];
    const char *filename;
} parser_t;

void parser_init(parser_t *parser, lexer_t *lexer);
ast_node_t *parser_parse(parser_t *parser);
ast_node_t *parser_parse_program(parser_t *parser);

ast_node_t *parse_declaration(parser_t *parser);
ast_node_t *parse_function(parser_t *parser, type_info_t *return_type, const char *name);
ast_node_t *parse_variable_declaration(parser_t *parser, type_info_t *type);
ast_node_t *parse_struct_declaration(parser_t *parser);
ast_node_t *parse_enum_declaration(parser_t *parser);
ast_node_t *parse_typedef(parser_t *parser);

ast_node_t *parse_statement(parser_t *parser);
ast_node_t *parse_block(parser_t *parser);
ast_node_t *parse_if_statement(parser_t *parser);
ast_node_t *parse_while_statement(parser_t *parser);
ast_node_t *parse_for_statement(parser_t *parser);
ast_node_t *parse_do_while_statement(parser_t *parser);
ast_node_t *parse_switch_statement(parser_t *parser);
ast_node_t *parse_return_statement(parser_t *parser);
ast_node_t *parse_break_statement(parser_t *parser);
ast_node_t *parse_continue_statement(parser_t *parser);

ast_node_t *parse_expression(parser_t *parser);
ast_node_t *parse_assignment_expression(parser_t *parser);
ast_node_t *parse_conditional_expression(parser_t *parser);
ast_node_t *parse_logical_or_expression(parser_t *parser);
ast_node_t *parse_logical_and_expression(parser_t *parser);
ast_node_t *parse_bitwise_or_expression(parser_t *parser);
ast_node_t *parse_bitwise_xor_expression(parser_t *parser);
ast_node_t *parse_bitwise_and_expression(parser_t *parser);
ast_node_t *parse_equality_expression(parser_t *parser);
ast_node_t *parse_relational_expression(parser_t *parser);
ast_node_t *parse_shift_expression(parser_t *parser);
ast_node_t *parse_additive_expression(parser_t *parser);
ast_node_t *parse_multiplicative_expression(parser_t *parser);
ast_node_t *parse_unary_expression(parser_t *parser);
ast_node_t *parse_postfix_expression(parser_t *parser);
ast_node_t *parse_primary_expression(parser_t *parser);
ast_node_t *parse_cast_expression(parser_t *parser);

type_info_t *parse_type_specifier(parser_t *parser);
type_info_t *parse_declarator(parser_t *parser, type_info_t *base_type, char *name);

ast_node_t *ast_create_node(ast_node_type_t type);
void ast_add_child(ast_node_t *parent, ast_node_t *child);
void ast_free(ast_node_t *node);
void ast_print(ast_node_t *node, int indent);

type_info_t *type_create(data_type_t base);
type_info_t *type_create_pointer(type_info_t *pointed);
type_info_t *type_create_array(type_info_t *element, int size);
type_info_t *type_copy(type_info_t *type);
void type_free(type_info_t *type);
bool type_equal(type_info_t *a, type_info_t *b);
bool type_compatible(type_info_t *a, type_info_t *b);
const char *type_to_string(type_info_t *type);
int type_size(type_info_t *type);

void parser_error(parser_t *parser, const char *message);
void parser_error_at(parser_t *parser, token_t *token, const char *message);
bool parser_had_error(parser_t *parser);
void parser_synchronize(parser_t *parser);

ast_node_t *parse_program(char *source);

#endif
