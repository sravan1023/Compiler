#ifndef _LEXER_H_
#define _LEXER_H_

#include "../include/kernel.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_TOKEN_LEN   256
#define MAX_STRING_LEN  1024

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_ERROR,
    TOKEN_NUMBER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_IDENTIFIER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,
    TOKEN_BIT_AND,
    TOKEN_BIT_OR,
    TOKEN_BIT_XOR,
    TOKEN_BIT_NOT,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,
    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_ASSIGN,
    TOKEN_PLUS_ASSIGN,
    TOKEN_MINUS_ASSIGN,
    TOKEN_MUL_ASSIGN,
    TOKEN_DIV_ASSIGN,
    TOKEN_MOD_ASSIGN,
    TOKEN_AND_ASSIGN,
    TOKEN_OR_ASSIGN,
    TOKEN_XOR_ASSIGN,
    TOKEN_LSHIFT_ASSIGN,
    TOKEN_RSHIFT_ASSIGN,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_ARROW,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_QUESTION,
    TOKEN_VOID,
    TOKEN_INT,
    TOKEN_CHAR_TYPE,
    TOKEN_FLOAT_TYPE,
    TOKEN_DOUBLE,
    TOKEN_LONG,
    TOKEN_SHORT,
    TOKEN_UNSIGNED,
    TOKEN_SIGNED,
    TOKEN_CONST,
    TOKEN_VOLATILE,
    TOKEN_STATIC,
    TOKEN_EXTERN,
    TOKEN_STRUCT,
    TOKEN_UNION,
    TOKEN_ENUM,
    TOKEN_TYPEDEF,
    TOKEN_SIZEOF,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_FOR,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_RETURN,
    TOKEN_GOTO,
    TOKEN_PROCESS,
    TOKEN_SYSCALL,
    TOKEN_INTERRUPT,
    TOKEN_SEMAPHORE,
    TOKEN_SIGNAL,
    TOKEN_WAIT,
    TOKEN_CREATE,
    TOKEN_RESUME,
    TOKEN_SUSPEND,
    TOKEN_KILL,
    TOKEN_SLEEP,
    TOKEN_YIELD,
    TOKEN_GETPID,
    TOKEN_GETPRIO,
    TOKEN_CHPRIO,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL_LITERAL
} token_type_t;

typedef struct {
    token_type_t type;
    char value[MAX_TOKEN_LEN];
    union {
        int64_t int_val;
        double float_val;
        char char_val;
    } literal;
    int line;
    int column;
    const char *filename;
} token_t;

typedef struct {
    const char *source;
    const char *filename;
    int pos;
    int line;
    int column;
    int length;
    bool has_error;
    char error_msg[256];
} lexer_t;

void lexer_init(char *source);
void lexer_init_ex(lexer_t *lexer, const char *source, const char *filename);
token_t lexer_next_token(void);
token_t lexer_next_token_ex(lexer_t *lexer);
token_t lexer_peek_token(void);
token_t lexer_peek_token_ex(lexer_t *lexer);
void lexer_unget_token(token_t token);

const char *token_type_to_string(token_type_t type);
bool token_is_type_keyword(token_type_t type);
bool token_is_storage_class(token_type_t type);
bool token_is_assignment_op(token_type_t type);
bool token_is_comparison_op(token_type_t type);
bool token_is_unary_op(token_type_t type);
bool token_is_binary_op(token_type_t type);
int token_get_precedence(token_type_t type);

void lexer_error(lexer_t *lexer, const char *msg);
bool lexer_has_error(lexer_t *lexer);
const char *lexer_get_error(lexer_t *lexer);

#endif


