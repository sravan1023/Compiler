#include "lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static lexer_t global_lexer;

static token_t peek_buffer;
static bool has_peek = false;
static token_t unget_buffer;
static bool has_unget = false;

typedef struct {
    const char *keyword;
    token_type_t type;
} keyword_entry_t;

static keyword_entry_t keywords[] = {
    {"void", TOKEN_VOID},
    {"int", TOKEN_INT},
    {"char", TOKEN_CHAR_TYPE},
    {"float", TOKEN_FLOAT_TYPE},
    {"double", TOKEN_DOUBLE},
    {"long", TOKEN_LONG},
    {"short", TOKEN_SHORT},
    {"unsigned", TOKEN_UNSIGNED},
    {"signed", TOKEN_SIGNED},
    {"const", TOKEN_CONST},
    {"volatile", TOKEN_VOLATILE},
    {"static", TOKEN_STATIC},
    {"extern", TOKEN_EXTERN},
    {"struct", TOKEN_STRUCT},
    {"union", TOKEN_UNION},
    {"enum", TOKEN_ENUM},
    {"typedef", TOKEN_TYPEDEF},
    {"sizeof", TOKEN_SIZEOF},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"do", TOKEN_DO},
    {"for", TOKEN_FOR},
    {"switch", TOKEN_SWITCH},
    {"case", TOKEN_CASE},
    {"default", TOKEN_DEFAULT},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"return", TOKEN_RETURN},
    {"goto", TOKEN_GOTO},
    {"process", TOKEN_PROCESS},
    {"syscall", TOKEN_SYSCALL},
    {"interrupt", TOKEN_INTERRUPT},
    {"semaphore", TOKEN_SEMAPHORE},
    {"signal", TOKEN_SIGNAL},
    {"wait", TOKEN_WAIT},
    {"create", TOKEN_CREATE},
    {"resume", TOKEN_RESUME},
    {"suspend", TOKEN_SUSPEND},
    {"kill", TOKEN_KILL},
    {"sleep", TOKEN_SLEEP},
    {"yield", TOKEN_YIELD},
    {"getpid", TOKEN_GETPID},
    {"getprio", TOKEN_GETPRIO},
    {"chprio", TOKEN_CHPRIO},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"null", TOKEN_NULL_LITERAL},
    {"NULL", TOKEN_NULL_LITERAL},
    {NULL, TOKEN_EOF}
};

void lexer_init(char *source) {
    lexer_init_ex(&global_lexer, source, "<stdin>");
    has_peek = false;
    has_unget = false;
}

void lexer_init_ex(lexer_t *lexer, const char *source, const char *filename) {
    lexer->source = source;
    lexer->filename = filename;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->length = strlen(source);
    lexer->has_error = false;
    lexer->error_msg[0] = '\0';
}

static char lexer_current(lexer_t *lexer) {
    if (lexer->pos >= lexer->length) {
        return '\0';
    }
    return lexer->source[lexer->pos];
}

static char lexer_peek(lexer_t *lexer) {
    if (lexer->pos + 1 >= lexer->length) {
        return '\0';
    }
    return lexer->source[lexer->pos + 1];
}

static char lexer_peek_at(lexer_t *lexer, int offset) {
    if (lexer->pos + offset >= lexer->length) {
        return '\0';
    }
    return lexer->source[lexer->pos + offset];
}

static void lexer_advance(lexer_t *lexer) {
    if (lexer->pos < lexer->length) {
        if (lexer->source[lexer->pos] == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        lexer->pos++;
    }
}

static void skip_whitespace(lexer_t *lexer) {
    while (lexer->pos < lexer->length) {
        char c = lexer_current(lexer);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            lexer_advance(lexer);
        } else {
            break;
        }
    }
}

static void skip_line_comment(lexer_t *lexer) {
    while (lexer->pos < lexer->length && lexer_current(lexer) != '\n') {
        lexer_advance(lexer);
    }
}

static void skip_block_comment(lexer_t *lexer) {
    lexer_advance(lexer);
    lexer_advance(lexer);
    
    while (lexer->pos < lexer->length) {
        if (lexer_current(lexer) == '*' && lexer_peek(lexer) == '/') {
            lexer_advance(lexer);
            lexer_advance(lexer);
            return;
        }
        lexer_advance(lexer);
    }
    
    lexer_error(lexer, "Unterminated block comment");
}

static void skip_whitespace_and_comments(lexer_t *lexer) {
    while (lexer->pos < lexer->length) {
        char c = lexer_current(lexer);
        
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            lexer_advance(lexer);
        } else if (c == '/' && lexer_peek(lexer) == '/') {
            skip_line_comment(lexer);
        } else if (c == '/' && lexer_peek(lexer) == '*') {
            skip_block_comment(lexer);
        } else {
            break;
        }
    }
}

static token_t make_token(lexer_t *lexer, token_type_t type, const char *value) {
    token_t token;
    token.type = type;
    strncpy(token.value, value, MAX_TOKEN_LEN - 1);
    token.value[MAX_TOKEN_LEN - 1] = '\0';
    token.line = lexer->line;
    token.column = lexer->column;
    token.filename = lexer->filename;
    token.literal.int_val = 0;
    return token;
}

/* Create error token */
static token_t make_error_token(lexer_t *lexer, const char *message) {
    token_t token;
    token.type = TOKEN_ERROR;
    strncpy(token.value, message, MAX_TOKEN_LEN - 1);
    token.value[MAX_TOKEN_LEN - 1] = '\0';
    token.line = lexer->line;
    token.column = lexer->column;
    token.filename = lexer->filename;
    lexer->has_error = true;
    strncpy(lexer->error_msg, message, sizeof(lexer->error_msg) - 1);
    return token;
}

static token_type_t lookup_keyword(const char *identifier) {
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (strcmp(keywords[i].keyword, identifier) == 0) {
            return keywords[i].type;
        }
    }
    return TOKEN_IDENTIFIER;
}

/* Read identifier */
static token_t read_identifier(lexer_t *lexer) {
    token_t token;
    int start = lexer->pos;
    int start_col = lexer->column;
    int len = 0;
    
    token.line = lexer->line;
    token.column = start_col;
    token.filename = lexer->filename;
    
    while (lexer->pos < lexer->length) {
        char c = lexer_current(lexer);
        if (isalnum(c) || c == '_') {
            if (len < MAX_TOKEN_LEN - 1) {
                token.value[len++] = c;
            }
            lexer_advance(lexer);
        } else {
            break;
        }
    }
    
    token.value[len] = '\0';
    token.type = lookup_keyword(token.value);
    token.literal.int_val = 0;
    
    return token;
}

static int hex_digit_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Read number (integer or float) */
static token_t read_number(lexer_t *lexer) {
    token_t token;
    int len = 0;
    bool is_float = false;
    bool is_hex = false;
    bool is_binary = false;
    bool is_octal = false;
    
    token.line = lexer->line;
    token.column = lexer->column;
    token.filename = lexer->filename;
    
    if (lexer_current(lexer) == '0') {
        token.value[len++] = lexer_current(lexer);
        lexer_advance(lexer);
        
        if (lexer_current(lexer) == 'x' || lexer_current(lexer) == 'X') {
            is_hex = true;
            token.value[len++] = lexer_current(lexer);
            lexer_advance(lexer);
        } else if (lexer_current(lexer) == 'b' || lexer_current(lexer) == 'B') {
            is_binary = true;
            token.value[len++] = lexer_current(lexer);
            lexer_advance(lexer);
        } else if (isdigit(lexer_current(lexer))) {
            is_octal = true;
        }
    }
    
    /* Read digits */
    while (lexer->pos < lexer->length && len < MAX_TOKEN_LEN - 1) {
        char c = lexer_current(lexer);
        
        if (is_hex) {
            if (isxdigit(c)) {
                token.value[len++] = c;
                lexer_advance(lexer);
            } else {
                break;
            }
        } else if (is_binary) {
            if (c == '0' || c == '1') {
                token.value[len++] = c;
                lexer_advance(lexer);
            } else {
                break;
            }
        } else if (isdigit(c)) {
            token.value[len++] = c;
            lexer_advance(lexer);
        } else if (c == '.' && !is_float && isdigit(lexer_peek(lexer))) {
            is_float = true;
            token.value[len++] = c;
            lexer_advance(lexer);
        } else if ((c == 'e' || c == 'E') && !is_hex) {
            is_float = true;
            token.value[len++] = c;
            lexer_advance(lexer);
            if (lexer_current(lexer) == '+' || lexer_current(lexer) == '-') {
                token.value[len++] = lexer_current(lexer);
                lexer_advance(lexer);
            }
        } else {
            break;
        }
    }
    
    while (lexer_current(lexer) == 'u' || lexer_current(lexer) == 'U' ||
           lexer_current(lexer) == 'l' || lexer_current(lexer) == 'L' ||
           lexer_current(lexer) == 'f' || lexer_current(lexer) == 'F') {
        lexer_advance(lexer);
    }
    
    token.value[len] = '\0';
    token.type = is_float ? TOKEN_FLOAT : TOKEN_NUMBER;
    
    if (is_float) {
        token.literal.float_val = atof(token.value);
    } else if (is_hex) {
        token.literal.int_val = strtoll(token.value, NULL, 16);
    } else if (is_binary) {
        token.literal.int_val = strtoll(token.value + 2, NULL, 2);
    } else if (is_octal) {
        token.literal.int_val = strtoll(token.value, NULL, 8);
    } else {
        token.literal.int_val = strtoll(token.value, NULL, 10);
    }
    
    return token;
}

static char read_escape_char(lexer_t *lexer) {
    lexer_advance(lexer);
    char c = lexer_current(lexer);
    lexer_advance(lexer);
    
    switch (c) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case '0':  return '\0';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"':  return '"';
        case 'a':  return '\a';
        case 'b':  return '\b';
        case 'f':  return '\f';
        case 'v':  return '\v';
        case 'x': {
            int value = 0;
            for (int i = 0; i < 2 && isxdigit(lexer_current(lexer)); i++) {
                value = value * 16 + hex_digit_value(lexer_current(lexer));
                lexer_advance(lexer);
            }
            return (char)value;
        }
        default:   return c;
    }
}

static token_t read_string(lexer_t *lexer) {
    token_t token;
    int len = 0;
    
    token.line = lexer->line;
    token.column = lexer->column;
    token.filename = lexer->filename;
    
    lexer_advance(lexer);  /* Skip opening quote */
    
    while (lexer->pos < lexer->length && len < MAX_TOKEN_LEN - 1) {
        char c = lexer_current(lexer);
        
        if (c == '"') {
            lexer_advance(lexer);
            break;
        } else if (c == '\\') {
            token.value[len++] = read_escape_char(lexer);
        } else if (c == '\n') {
            return make_error_token(lexer, "Unterminated string literal");
        } else {
            token.value[len++] = c;
            lexer_advance(lexer);
        }
    }
    
    token.value[len] = '\0';
    token.type = TOKEN_STRING;
    
    return token;
}

static token_t read_char(lexer_t *lexer) {
    token_t token;
    
    token.line = lexer->line;
    token.column = lexer->column;
    token.filename = lexer->filename;
    
    lexer_advance(lexer);  /* Skip opening quote */
    
    if (lexer_current(lexer) == '\\') {
        token.literal.char_val = read_escape_char(lexer);
    } else {
        token.literal.char_val = lexer_current(lexer);
        lexer_advance(lexer);
    }
    
    if (lexer_current(lexer) != '\'') {
        return make_error_token(lexer, "Unterminated character literal");
    }
    
    lexer_advance(lexer);
    
    token.value[0] = token.literal.char_val;
    token.value[1] = '\0';
    token.type = TOKEN_CHAR;
    
    return token;
}

token_t lexer_next_token_ex(lexer_t *lexer) {
    token_t token;
    char c;
    
    /* Check unget buffer first */
    if (has_unget) {
        has_unget = false;
        return unget_buffer;
    }
    
    skip_whitespace_and_comments(lexer);
    
    if (lexer->pos >= lexer->length) {
        return make_token(lexer, TOKEN_EOF, "");
    }
    
    c = lexer_current(lexer);
    int start_line = lexer->line;
    int start_col = lexer->column;
    
    if (isalpha(c) || c == '_') {
        return read_identifier(lexer);
    }
    
    if (isdigit(c)) {
        return read_number(lexer);
    }
    
    if (c == '"') {
        return read_string(lexer);
    }
    
    if (c == '\'') {
        return read_char(lexer);
    }
    
    /* Operators and delimiters */
    lexer_advance(lexer);
    
    switch (c) {
        case '+':
            if (lexer_current(lexer) == '+') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_INCREMENT, "++");
            } else if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_PLUS_ASSIGN, "+=");
            }
            return make_token(lexer, TOKEN_PLUS, "+");
            
        case '-':
            if (lexer_current(lexer) == '-') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_DECREMENT, "--");
            } else if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_MINUS_ASSIGN, "-=");
            } else if (lexer_current(lexer) == '>') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_ARROW, "->");
            }
            return make_token(lexer, TOKEN_MINUS, "-");
            
        case '*':
            if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_MUL_ASSIGN, "*=");
            }
            return make_token(lexer, TOKEN_MULTIPLY, "*");
            
        case '/':
            if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_DIV_ASSIGN, "/=");
            }
            return make_token(lexer, TOKEN_DIVIDE, "/");
            
        case '%':
            if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_MOD_ASSIGN, "%=");
            }
            return make_token(lexer, TOKEN_MODULO, "%");
            
        case '&':
            if (lexer_current(lexer) == '&') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_AND, "&&");
            } else if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_AND_ASSIGN, "&=");
            }
            return make_token(lexer, TOKEN_BIT_AND, "&");
            
        case '|':
            if (lexer_current(lexer) == '|') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_OR, "||");
            } else if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_OR_ASSIGN, "|=");
            }
            return make_token(lexer, TOKEN_BIT_OR, "|");
            
        case '^':
            if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_XOR_ASSIGN, "^=");
            }
            return make_token(lexer, TOKEN_BIT_XOR, "^");
            
        case '~':
            return make_token(lexer, TOKEN_BIT_NOT, "~");
            
        case '!':
            if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_NE, "!=");
            }
            return make_token(lexer, TOKEN_NOT, "!");
            
        case '=':
            if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_EQ, "==");
            }
            return make_token(lexer, TOKEN_ASSIGN, "=");
            
        case '<':
            if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_LE, "<=");
            } else if (lexer_current(lexer) == '<') {
                lexer_advance(lexer);
                if (lexer_current(lexer) == '=') {
                    lexer_advance(lexer);
                    return make_token(lexer, TOKEN_LSHIFT_ASSIGN, "<<=");
                }
                return make_token(lexer, TOKEN_LSHIFT, "<<");
            }
            return make_token(lexer, TOKEN_LT, "<");
            
        case '>':
            if (lexer_current(lexer) == '=') {
                lexer_advance(lexer);
                return make_token(lexer, TOKEN_GE, ">=");
            } else if (lexer_current(lexer) == '>') {
                lexer_advance(lexer);
                if (lexer_current(lexer) == '=') {
                    lexer_advance(lexer);
                    return make_token(lexer, TOKEN_RSHIFT_ASSIGN, ">>=");
                }
                return make_token(lexer, TOKEN_RSHIFT, ">>");
            }
            return make_token(lexer, TOKEN_GT, ">");
            
        case ';':
            return make_token(lexer, TOKEN_SEMICOLON, ";");
            
        case ':':
            return make_token(lexer, TOKEN_COLON, ":");
            
        case ',':
            return make_token(lexer, TOKEN_COMMA, ",");
            
        case '.':
            if (isdigit(lexer_current(lexer))) {
                /* Decimal number starting with . */
                lexer->pos--;
                lexer->column--;
                return read_number(lexer);
            }
            return make_token(lexer, TOKEN_DOT, ".");
            
        case '(':
            return make_token(lexer, TOKEN_LPAREN, "(");
            
        case ')':
            return make_token(lexer, TOKEN_RPAREN, ")");
            
        case '{':
            return make_token(lexer, TOKEN_LBRACE, "{");
            
        case '}':
            return make_token(lexer, TOKEN_RBRACE, "}");
            
        case '[':
            return make_token(lexer, TOKEN_LBRACKET, "[");
            
        case ']':
            return make_token(lexer, TOKEN_RBRACKET, "]");
            
        case '?':
            return make_token(lexer, TOKEN_QUESTION, "?");
            
        default: {
            char msg[64];
            snprintf(msg, sizeof(msg), "Unexpected character: '%c'", c);
            return make_error_token(lexer, msg);
        }
    }
}

token_t lexer_next_token(void) {
    return lexer_next_token_ex(&global_lexer);
}

token_t lexer_peek_token_ex(lexer_t *lexer) {
    if (!has_peek) {
        peek_buffer = lexer_next_token_ex(lexer);
        has_peek = true;
    }
    return peek_buffer;
}

token_t lexer_peek_token(void) {
    return lexer_peek_token_ex(&global_lexer);
}

/* Put token back */
void lexer_unget_token(token_t token) {
    unget_buffer = token;
    has_unget = true;
}

const char *token_type_to_string(token_type_t type) {
    switch (type) {
        case TOKEN_EOF:         return "EOF";
        case TOKEN_ERROR:       return "ERROR";
        case TOKEN_NUMBER:      return "NUMBER";
        case TOKEN_FLOAT:       return "FLOAT";
        case TOKEN_STRING:      return "STRING";
        case TOKEN_CHAR:        return "CHAR";
        case TOKEN_IDENTIFIER:  return "IDENTIFIER";
        case TOKEN_PLUS:        return "+";
        case TOKEN_MINUS:       return "-";
        case TOKEN_MULTIPLY:    return "*";
        case TOKEN_DIVIDE:      return "/";
        case TOKEN_MODULO:      return "%";
        case TOKEN_INCREMENT:   return "++";
        case TOKEN_DECREMENT:   return "--";
        case TOKEN_BIT_AND:     return "&";
        case TOKEN_BIT_OR:      return "|";
        case TOKEN_BIT_XOR:     return "^";
        case TOKEN_BIT_NOT:     return "~";
        case TOKEN_LSHIFT:      return "<<";
        case TOKEN_RSHIFT:      return ">>";
        case TOKEN_EQ:          return "==";
        case TOKEN_NE:          return "!=";
        case TOKEN_LT:          return "<";
        case TOKEN_GT:          return ">";
        case TOKEN_LE:          return "<=";
        case TOKEN_GE:          return ">=";
        case TOKEN_AND:         return "&&";
        case TOKEN_OR:          return "||";
        case TOKEN_NOT:         return "!";
        case TOKEN_ASSIGN:      return "=";
        case TOKEN_SEMICOLON:   return ";";
        case TOKEN_COLON:       return ":";
        case TOKEN_COMMA:       return ",";
        case TOKEN_DOT:         return ".";
        case TOKEN_ARROW:       return "->";
        case TOKEN_LPAREN:      return "(";
        case TOKEN_RPAREN:      return ")";
        case TOKEN_LBRACE:      return "{";
        case TOKEN_RBRACE:      return "}";
        case TOKEN_LBRACKET:    return "[";
        case TOKEN_RBRACKET:    return "]";
        case TOKEN_IF:          return "if";
        case TOKEN_ELSE:        return "else";
        case TOKEN_WHILE:       return "while";
        case TOKEN_FOR:         return "for";
        case TOKEN_DO:          return "do";
        case TOKEN_RETURN:      return "return";
        case TOKEN_BREAK:       return "break";
        case TOKEN_CONTINUE:    return "continue";
        case TOKEN_SWITCH:      return "switch";
        case TOKEN_CASE:        return "case";
        case TOKEN_DEFAULT:     return "default";
        case TOKEN_VOID:        return "void";
        case TOKEN_INT:         return "int";
        case TOKEN_CHAR_TYPE:   return "char";
        case TOKEN_FLOAT_TYPE:  return "float";
        case TOKEN_STRUCT:      return "struct";
        case TOKEN_PROCESS:     return "process";
        case TOKEN_SYSCALL:     return "syscall";
        default:                return "UNKNOWN";
    }
}

/* Check if token is a type keyword */
bool token_is_type_keyword(token_type_t type) {
    return type == TOKEN_VOID || type == TOKEN_INT || type == TOKEN_CHAR_TYPE ||
           type == TOKEN_FLOAT_TYPE || type == TOKEN_DOUBLE || type == TOKEN_LONG ||
           type == TOKEN_SHORT || type == TOKEN_UNSIGNED || type == TOKEN_SIGNED ||
           type == TOKEN_STRUCT || type == TOKEN_UNION || type == TOKEN_ENUM;
}

/* Check if token is a storage class */
bool token_is_storage_class(token_type_t type) {
    return type == TOKEN_STATIC || type == TOKEN_EXTERN ||
           type == TOKEN_CONST || type == TOKEN_VOLATILE;
}

bool token_is_assignment_op(token_type_t type) {
    return type == TOKEN_ASSIGN || type == TOKEN_PLUS_ASSIGN ||
           type == TOKEN_MINUS_ASSIGN || type == TOKEN_MUL_ASSIGN ||
           type == TOKEN_DIV_ASSIGN || type == TOKEN_MOD_ASSIGN ||
           type == TOKEN_AND_ASSIGN || type == TOKEN_OR_ASSIGN ||
           type == TOKEN_XOR_ASSIGN || type == TOKEN_LSHIFT_ASSIGN ||
           type == TOKEN_RSHIFT_ASSIGN;
}

/* Check if token is a comparison operator */
bool token_is_comparison_op(token_type_t type) {
    return type == TOKEN_EQ || type == TOKEN_NE ||
           type == TOKEN_LT || type == TOKEN_GT ||
           type == TOKEN_LE || type == TOKEN_GE;
}

bool token_is_unary_op(token_type_t type) {
    return type == TOKEN_PLUS || type == TOKEN_MINUS ||
           type == TOKEN_NOT || type == TOKEN_BIT_NOT ||
           type == TOKEN_INCREMENT || type == TOKEN_DECREMENT ||
           type == TOKEN_MULTIPLY || type == TOKEN_BIT_AND;
}

bool token_is_binary_op(token_type_t type) {
    return type == TOKEN_PLUS || type == TOKEN_MINUS ||
           type == TOKEN_MULTIPLY || type == TOKEN_DIVIDE ||
           type == TOKEN_MODULO || type == TOKEN_BIT_AND ||
           type == TOKEN_BIT_OR || type == TOKEN_BIT_XOR ||
           type == TOKEN_LSHIFT || type == TOKEN_RSHIFT ||
           type == TOKEN_AND || type == TOKEN_OR ||
           token_is_comparison_op(type);
}

/* Get operator precedence */
int token_get_precedence(token_type_t type) {
    switch (type) {
        case TOKEN_OR:          return 1;
        case TOKEN_AND:         return 2;
        case TOKEN_BIT_OR:      return 3;
        case TOKEN_BIT_XOR:     return 4;
        case TOKEN_BIT_AND:     return 5;
        case TOKEN_EQ:
        case TOKEN_NE:          return 6;
        case TOKEN_LT:
        case TOKEN_GT:
        case TOKEN_LE:
        case TOKEN_GE:          return 7;
        case TOKEN_LSHIFT:
        case TOKEN_RSHIFT:      return 8;
        case TOKEN_PLUS:
        case TOKEN_MINUS:       return 9;
        case TOKEN_MULTIPLY:
        case TOKEN_DIVIDE:
        case TOKEN_MODULO:      return 10;
        default:                return 0;
    }
}

void lexer_error(lexer_t *lexer, const char *msg) {
    lexer->has_error = true;
    snprintf(lexer->error_msg, sizeof(lexer->error_msg),
             "%s:%d:%d: error: %s",
             lexer->filename, lexer->line, lexer->column, msg);
}

bool lexer_has_error(lexer_t *lexer) {
    return lexer->has_error;
}

const char *lexer_get_error(lexer_t *lexer) {
    return lexer->error_msg;
}

