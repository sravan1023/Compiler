#ifndef _PARSER_H_
#define _PARSER_H_

#include "lexer.h"
#include <stdint.h>
#include <stdbool.h>

/* Maximum limits */
#define MAX_PARAMS          32
#define MAX_LOCALS          256
#define MAX_STRUCT_MEMBERS  64
#define MAX_ENUM_VALUES     128
#define MAX_ARRAY_DIMS      8

/* AST Node types */
typedef enum {
    /* Program structure */
    NODE_PROGRAM,           /* Root program node */
    NODE_FUNCTION,          /* Function definition */
    NODE_PROCESS,           /* Xinu process definition */
    NODE_SYSCALL,           /* System call definition */
    NODE_INTERRUPT,         /* Interrupt handler */
    NODE_PARAM,             /* Function parameter */
    NODE_BLOCK,             /* Statement block */
    
    /* Declarations */
    NODE_VAR_DECL,          /* Variable declaration */
    NODE_ARRAY_DECL,        /* Array declaration */
    NODE_STRUCT_DECL,       /* Struct declaration */
    NODE_UNION_DECL,        /* Union declaration */
    NODE_ENUM_DECL,         /* Enum declaration */
    NODE_TYPEDEF,           /* Type definition */
    NODE_FIELD,             /* Struct/union field */
    
    /* Statements */
    NODE_EXPR_STMT,         /* Expression statement */
    NODE_IF,                /* If statement */
    NODE_WHILE,             /* While loop */
    NODE_DO_WHILE,          /* Do-while loop */
    NODE_FOR,               /* For loop */
    NODE_SWITCH,            /* Switch statement */
    NODE_CASE,              /* Case label */
    NODE_DEFAULT,           /* Default label */
    NODE_RETURN,            /* Return statement */
    NODE_BREAK,             /* Break statement */
    NODE_CONTINUE,          /* Continue statement */
    NODE_GOTO,              /* Goto statement */
    NODE_LABEL,             /* Label statement */
    NODE_EMPTY,             /* Empty statement */
    
    /* Expressions */
    NODE_NUMBER,            /* Integer literal */
    NODE_FLOAT,             /* Float literal */
    NODE_STRING,            /* String literal */
    NODE_CHAR,              /* Character literal */
    NODE_IDENTIFIER,        /* Variable reference */
    NODE_BINARY_OP,         /* Binary operation */
    NODE_UNARY_OP,          /* Unary operation */
    NODE_ASSIGN,            /* Assignment */
    NODE_COMPOUND_ASSIGN,   /* Compound assignment (+=, etc.) */
    NODE_TERNARY,           /* Ternary operator ?: */
    NODE_CALL,              /* Function call */
    NODE_ARRAY_ACCESS,      /* Array indexing */
    NODE_MEMBER_ACCESS,     /* Struct member access */
    NODE_PTR_MEMBER,        /* Pointer member access -> */
    NODE_CAST,              /* Type cast */
    NODE_SIZEOF,            /* Sizeof expression */
    NODE_ADDRESS_OF,        /* Address-of operator & */
    NODE_DEREFERENCE,       /* Pointer dereference * */
    NODE_PRE_INC,           /* Pre-increment ++x */
    NODE_PRE_DEC,           /* Pre-decrement --x */
    NODE_POST_INC,          /* Post-increment x++ */
    NODE_POST_DEC,          /* Post-decrement x-- */
    NODE_COMMA,             /* Comma expression */
    NODE_INIT_LIST,         /* Initializer list */
    
    /* Xinu-specific */
    NODE_CREATE,            /* Create process */
    NODE_RESUME,            /* Resume process */
    NODE_SUSPEND,           /* Suspend process */
    NODE_KILL,              /* Kill process */
    NODE_SLEEP,             /* Sleep */
    NODE_YIELD,             /* Yield CPU */
    NODE_WAIT,              /* Wait on semaphore */
    NODE_SIGNAL,            /* Signal semaphore */
    NODE_GETPID,            /* Get process ID */
    NODE_SEMAPHORE,         /* Semaphore declaration */
    
    /* Type nodes */
    NODE_TYPE,              /* Type specifier */
    NODE_POINTER_TYPE,      /* Pointer type */
    NODE_ARRAY_TYPE,        /* Array type */
    NODE_FUNC_TYPE          /* Function pointer type */
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

/* Type qualifiers */
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

/* Forward declarations */
struct ast_node;
struct type_info;
struct symbol;

/* Type information */
typedef struct type_info {
    data_type_t base_type;          /* Base type */
    uint32_t qualifiers;            /* Type qualifiers bitmap */
    int pointer_depth;              /* Number of pointer indirections */
    int array_dims;                 /* Number of array dimensions */
    int array_sizes[MAX_ARRAY_DIMS];/* Size of each dimension */
    char struct_name[MAX_TOKEN_LEN];/* Name for struct/union/enum */
    struct type_info *pointed_type; /* For pointers: what we point to */
    struct type_info *return_type;  /* For functions: return type */
    int num_params;                 /* For functions: parameter count */
    struct type_info **param_types; /* For functions: parameter types */
} type_info_t;

/* AST Node structure */
typedef struct ast_node {
    ast_node_type_t type;           /* Node type */
    token_t token;                  /* Associated token */
    type_info_t *data_type;         /* Data type for expressions */
    
    /* Value storage */
    union {
        int64_t int_val;            /* Integer value */
        double float_val;           /* Float value */
        char char_val;              /* Character value */
        char *string_val;           /* String value */
    } value;
    
    /* Node-specific data */
    char name[MAX_TOKEN_LEN];       /* Name for identifiers/functions */
    char op[8];                     /* Operator string */
    
    /* Child nodes */
    struct ast_node *left;          /* Left child / condition / base */
    struct ast_node *right;         /* Right child / increment */
    struct ast_node *extra;         /* Extra child for ternary, etc. */
    
    /* For blocks and lists */
    struct ast_node **children;     /* Array of child nodes */
    int child_count;                /* Number of children */
    int child_capacity;             /* Capacity of children array */
    
    /* Source location */
    int line;                       /* Line number */
    int column;                     /* Column number */
    const char *filename;           /* Source file */
    
    /* For semantic analysis */
    struct symbol *symbol;          /* Associated symbol */
    bool is_lvalue;                 /* Is this an lvalue? */
    bool is_constant;               /* Is this a constant expression? */
} ast_node_t;

/* Parser state */
typedef struct {
    lexer_t *lexer;                 /* Lexer instance */
    token_t current;                /* Current token */
    token_t previous;               /* Previous token */
    bool had_error;                 /* Error flag */
    bool panic_mode;                /* Panic mode for error recovery */
    char error_msg[512];            /* Error message */
    const char *filename;           /* Current filename */
} parser_t;

/* Parser functions */
void parser_init(parser_t *parser, lexer_t *lexer);
ast_node_t *parser_parse(parser_t *parser);
ast_node_t *parser_parse_program(parser_t *parser);

/* Declaration parsing */
ast_node_t *parse_declaration(parser_t *parser);
ast_node_t *parse_function(parser_t *parser, type_info_t *return_type, const char *name);
ast_node_t *parse_variable_declaration(parser_t *parser, type_info_t *type);
ast_node_t *parse_struct_declaration(parser_t *parser);
ast_node_t *parse_enum_declaration(parser_t *parser);
ast_node_t *parse_typedef(parser_t *parser);

/* Statement parsing */
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

/* Expression parsing */
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

/* Type parsing */
type_info_t *parse_type_specifier(parser_t *parser);
type_info_t *parse_declarator(parser_t *parser, type_info_t *base_type, char *name);

/* AST utilities */
ast_node_t *ast_create_node(ast_node_type_t type);
void ast_add_child(ast_node_t *parent, ast_node_t *child);
void ast_free(ast_node_t *node);
void ast_print(ast_node_t *node, int indent);

/* Type utilities */
type_info_t *type_create(data_type_t base);
type_info_t *type_create_pointer(type_info_t *pointed);
type_info_t *type_create_array(type_info_t *element, int size);
type_info_t *type_copy(type_info_t *type);
void type_free(type_info_t *type);
bool type_equal(type_info_t *a, type_info_t *b);
bool type_compatible(type_info_t *a, type_info_t *b);
const char *type_to_string(type_info_t *type);
int type_size(type_info_t *type);

/* Error handling */
void parser_error(parser_t *parser, const char *message);
void parser_error_at(parser_t *parser, token_t *token, const char *message);
bool parser_had_error(parser_t *parser);
void parser_synchronize(parser_t *parser);


ast_node_t *parse_program(char *source);

#endif
