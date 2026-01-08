#ifndef _LEXER_H_
#define _LEXER_H_

#include "../include/kernel.h"
#include <stdint.h>
#include <stdbool.h>

/* Maximum identifier/string length */
#define MAX_TOKEN_LEN   256
#define MAX_STRING_LEN  1024

/* Token types */
typedef enum {
    /* End of file and error */
    TOKEN_EOF = 0,
    TOKEN_ERROR,
    
    /* Literals */
    TOKEN_NUMBER,           /* Integer literal: 123, 0xFF, 0b101 */
    TOKEN_FLOAT,            /* Float literal: 3.14, 1.0e-5 */
    TOKEN_STRING,           /* String literal: "hello" */
    TOKEN_CHAR,             /* Character literal: 'a' */
    TOKEN_IDENTIFIER,       /* Variable/function name */
    
    /* Arithmetic operators */
    TOKEN_PLUS,             /* + */
    TOKEN_MINUS,            /* - */
    TOKEN_MULTIPLY,         /* * */
    TOKEN_DIVIDE,           /* / */
    TOKEN_MODULO,           /* % */
    TOKEN_INCREMENT,        /* ++ */
    TOKEN_DECREMENT,        /* -- */
    
    /* Bitwise operators */
    TOKEN_BIT_AND,          /* & */
    TOKEN_BIT_OR,           /* | */
    TOKEN_BIT_XOR,          /* ^ */
    TOKEN_BIT_NOT,          /* ~ */
    TOKEN_LSHIFT,           /* << */
    TOKEN_RSHIFT,           /* >> */
    
    /* Comparison operators */
    TOKEN_EQ,               /* == */
    TOKEN_NE,               /* != */
    TOKEN_LT,               /* < */
    TOKEN_GT,               /* > */
    TOKEN_LE,               /* <= */
    TOKEN_GE,               /* >= */
    
    /* Logical operators */
    TOKEN_AND,              /* && */
    TOKEN_OR,               /* || */
    TOKEN_NOT,              /* ! */
    
    /* Assignment operators */
    TOKEN_ASSIGN,           /* = */
    TOKEN_PLUS_ASSIGN,      /* += */
    TOKEN_MINUS_ASSIGN,     /* -= */
    TOKEN_MUL_ASSIGN,       /* *= */
    TOKEN_DIV_ASSIGN,       /* /= */
    TOKEN_MOD_ASSIGN,       /* %= */
    TOKEN_AND_ASSIGN,       /* &= */
    TOKEN_OR_ASSIGN,        /* |= */
    TOKEN_XOR_ASSIGN,       /* ^= */
    TOKEN_LSHIFT_ASSIGN,    /* <<= */
    TOKEN_RSHIFT_ASSIGN,    /* >>= */
    
    /* Delimiters */
    TOKEN_SEMICOLON,        /* ; */
    TOKEN_COLON,            /* : */
    TOKEN_COMMA,            /* , */
    TOKEN_DOT,              /* . */
    TOKEN_ARROW,            /* -> */
    TOKEN_LPAREN,           /* ( */
    TOKEN_RPAREN,           /* ) */
    TOKEN_LBRACE,           /* { */
    TOKEN_RBRACE,           /* } */
    TOKEN_LBRACKET,         /* [ */
    TOKEN_RBRACKET,         /* ] */
    TOKEN_QUESTION,         /* ? */
    
    /* Keywords - Types */
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
    
    /* Keywords - Control flow */
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
    
    /* Xinu-specific keywords */
    TOKEN_PROCESS,          /* Process declaration */
    TOKEN_SYSCALL,          /* System call */
    TOKEN_INTERRUPT,        /* Interrupt handler */
    TOKEN_SEMAPHORE,        /* Semaphore declaration */
    TOKEN_SIGNAL,           /* Signal operation */
    TOKEN_WAIT,             /* Wait operation */
    TOKEN_CREATE,           /* Create process */
    TOKEN_RESUME,           /* Resume process */
    TOKEN_SUSPEND,          /* Suspend process */
    TOKEN_KILL,             /* Kill process */
    TOKEN_SLEEP,            /* Sleep */
    TOKEN_YIELD,            /* Yield CPU */
    TOKEN_GETPID,           /* Get process ID */
    TOKEN_GETPRIO,          /* Get priority */
    TOKEN_CHPRIO,           /* Change priority */
    
    /* Boolean literals */
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL_LITERAL
} token_type_t;

/* Token structure */
typedef struct {
    token_type_t type;              /* Token type */
    char value[MAX_TOKEN_LEN];      /* Token value as string */
    union {
        int64_t int_val;            /* Integer value */
        double float_val;           /* Float value */
        char char_val;              /* Character value */
    } literal;
    int line;                       /* Line number */
    int column;                     /* Column number */
    const char *filename;           /* Source filename */
} token_t;

/* Lexer state structure */
typedef struct {
    const char *source;             /* Source code */
    const char *filename;           /* Source filename */
    int pos;                        /* Current position */
    int line;                       /* Current line */
    int column;                     /* Current column */
    int length;                     /* Source length */
    bool has_error;                 /* Error flag */
    char error_msg[256];            /* Error message */
} lexer_t;

/* Lexer functions */
void lexer_init(char *source);
void lexer_init_ex(lexer_t *lexer, const char *source, const char *filename);
token_t lexer_next_token(void);
token_t lexer_next_token_ex(lexer_t *lexer);
token_t lexer_peek_token(void);
token_t lexer_peek_token_ex(lexer_t *lexer);
void lexer_unget_token(token_t token);

/* Token utility functions */
const char *token_type_to_string(token_type_t type);
bool token_is_type_keyword(token_type_t type);
bool token_is_storage_class(token_type_t type);
bool token_is_assignment_op(token_type_t type);
bool token_is_comparison_op(token_type_t type);
bool token_is_unary_op(token_type_t type);
bool token_is_binary_op(token_type_t type);
int token_get_precedence(token_type_t type);

/* Error handling */
void lexer_error(lexer_t *lexer, const char *msg);
bool lexer_has_error(lexer_t *lexer);
const char *lexer_get_error(lexer_t *lexer);

#endif


