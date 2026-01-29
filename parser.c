#include "parser.h"
#include "../include/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static parser_t global_parser;

static void advance(parser_t *parser);
static bool match(parser_t *parser, token_type_t type);
static bool check(parser_t *parser, token_type_t type);
static void consume(parser_t *parser, token_type_t type, const char *message);

void parser_init(parser_t *parser, lexer_t *lexer) {
    parser->lexer = lexer;
    parser->had_error = false;
    parser->panic_mode = false;
    parser->error_msg[0] = '\0';
    parser->filename = lexer->filename;
    advance(parser);
}

static void advance(parser_t *parser) {
    parser->previous = parser->current;
    
    for (;;) {
        parser->current = lexer_next_token_ex(parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;
        
        parser_error_at(parser, &parser->current, parser->current.value);
    }
}

static bool check(parser_t *parser, token_type_t type) {
    return parser->current.type == type;
}

static bool match(parser_t *parser, token_type_t type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}

/* Consume token or error */
static void consume(parser_t *parser, token_type_t type, const char *message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    parser_error_at(parser, &parser->current, message);
        return node;
    } else if (current_token.type == TOKEN_IDENTIFIER) {
        node = alloc_node();
        node->type = NODE_IDENTIFIER;
        strcpy(node->value, current_token.value);
        current_token = lexer_next_token();
        return node;
    } else if (current_token.type == TOKEN_LPAREN) {
        current_token = lexer_next_token();
        node = parse_expression();
        if (current_token.type != TOKEN_RPAREN) {
            /* Error: expected ')' */
            return NULL;
        }
        current_token = lexer_next_token();
        return node;
    }
    
    return NULL;
}

ast_node_t *parse_term(void) {
    ast_node_t *node = parse_factor();
    ast_node_t *new_node;
    
    while (current_token.type == TOKEN_MULTIPLY || 
           current_token.type == TOKEN_DIVIDE) {
        new_node = alloc_node();
        new_node->type = NODE_BINARY_OP;
        new_node->left = node;
        if (current_token.type == TOKEN_MULTIPLY) {
            strcpy(new_node->value, "*");
        } else {
            strcpy(new_node->value, "/");
        }
        current_token = lexer_next_token();
        new_node->right = parse_factor();
        node = new_node;
    }
    
    return node;
}

ast_node_t *parse_expression(void) {
    ast_node_t *node = parse_term();
    ast_node_t *new_node;
    
    while (current_token.type == TOKEN_PLUS || 
           current_token.type == TOKEN_MINUS) {
        new_node = alloc_node();
        new_node->type = NODE_BINARY_OP;
        new_node->left = node;
        if (current_token.type == TOKEN_PLUS) {
            strcpy(new_node->value, "+");
        } else {
            strcpy(new_node->value, "-");
        }
        current_token = lexer_next_token();
        new_node->right = parse_term();
        node = new_node;
    }
    
    return node;
}

/* Parse assignment statement */
ast_node_t *parse_assignment(void) {
    ast_node_t *node;
    
    if (current_token.type != TOKEN_IDENTIFIER) {
        return NULL;
    }
    
    node = alloc_node();
    node->type = NODE_IDENTIFIER;
    strcpy(node->value, current_token.value);
    current_token = lexer_next_token();
    
    if (current_token.type != TOKEN_ASSIGN) {
        return NULL;
    }
    
    current_token = lexer_next_token();
    node->type = NODE_ASSIGN;
    node->left = node;
    node->right = parse_expression();
    
    return node;
}

ast_node_t *parse_statement(void) {
    ast_node_t *node;
    
    if (current_token.type == TOKEN_IF) {
        node = alloc_node();
        node->type = NODE_IF;
        current_token = lexer_next_token();
        /* Simplified - would need more parsing */
        return node;
    } else if (current_token.type == TOKEN_WHILE) {
        node = alloc_node();
        node->type = NODE_WHILE;
        current_token = lexer_next_token();
        /* Simplified - would need more parsing */
        return node;
    } else if (current_token.type == TOKEN_RETURN) {
        node = alloc_node();
        node->type = NODE_RETURN;
        current_token = lexer_next_token();
        node->left = parse_expression();
        return node;
    } else {
        return parse_assignment();
    }
}

ast_node_t *parse_program(char *source) {
    ast_node_t *node;
    
    lexer_init(source);
    current_token = lexer_next_token();
    
    node = parse_statement();
    
}

/* AST Node Creation */

ast_node_t *ast_create_node(ast_node_type_t type) {
    ast_node_t *node = (ast_node_t *)getmem(sizeof(ast_node_t));
    if (node == NULL) return NULL;
    
    memset(node, 0, sizeof(ast_node_t));
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->extra = NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    node->data_type = NULL;
    node->symbol = NULL;
    node->is_lvalue = false;
    node->is_constant = false;
    
    return node;
}

void ast_add_child(ast_node_t *parent, ast_node_t *child) {
    if (parent == NULL || child == NULL) return;
    
    if (parent->child_count >= parent->child_capacity) {
        int new_capacity = parent->child_capacity == 0 ? 8 : parent->child_capacity * 2;
        ast_node_t **new_children = (ast_node_t **)getmem(sizeof(ast_node_t *) * new_capacity);
        
        if (parent->children != NULL) {
            memcpy(new_children, parent->children, sizeof(ast_node_t *) * parent->child_count);
            freemem(parent->children, sizeof(ast_node_t *) * parent->child_capacity);
        }
        
        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }
    
    parent->children[parent->child_count++] = child;
}

void ast_free(ast_node_t *node) {
    if (node == NULL) return;
    
    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->extra);
    
    for (int i = 0; i < node->child_count; i++) {
        ast_free(node->children[i]);
    }
    
    if (node->children != NULL) {
        freemem(node->children, sizeof(ast_node_t *) * node->child_capacity);
    }
    
    if (node->data_type != NULL) {
        type_free(node->data_type);
    }
    
    freemem(node, sizeof(ast_node_t));
}

type_info_t *type_create(data_type_t base) {
    type_info_t *type = (type_info_t *)getmem(sizeof(type_info_t));
    if (type == NULL) return NULL;
    
    memset(type, 0, sizeof(type_info_t));
    type->base_type = base;
    type->qualifiers = QUAL_NONE;
    type->pointer_depth = 0;
    type->array_dims = 0;
    type->pointed_type = NULL;
    type->return_type = NULL;
    type->num_params = 0;
    type->param_types = NULL;
    
    return type;
}

type_info_t *type_create_pointer(type_info_t *pointed) {
    type_info_t *type = type_create(TYPE_POINTER);
    if (type != NULL) {
        type->pointed_type = pointed;
        type->pointer_depth = 1;
    }
    return type;
}

type_info_t *type_create_array(type_info_t *element, int size) {
    type_info_t *type = type_copy(element);
    if (type != NULL && type->array_dims < MAX_ARRAY_DIMS) {
        type->array_sizes[type->array_dims++] = size;
    }
    return type;
}

type_info_t *type_copy(type_info_t *type) {
    if (type == NULL) return NULL;
    
    type_info_t *copy = (type_info_t *)getmem(sizeof(type_info_t));
    if (copy != NULL) {
        memcpy(copy, type, sizeof(type_info_t));
    }
    return copy;
}

void type_free(type_info_t *type) {
    if (type == NULL) return;
    
    if (type->pointed_type != NULL) {
        type_free(type->pointed_type);
    }
    
    if (type->return_type != NULL) {
        type_free(type->return_type);
    }
    
    if (type->param_types != NULL) {
        for (int i = 0; i < type->num_params; i++) {
            type_free(type->param_types[i]);
        }
        freemem(type->param_types, sizeof(type_info_t *) * type->num_params);
    }
    
    freemem(type, sizeof(type_info_t));
}

bool type_equal(type_info_t *a, type_info_t *b) {
    if (a == NULL || b == NULL) return a == b;
    if (a->base_type != b->base_type) return false;
    if (a->pointer_depth != b->pointer_depth) return false;
    if (a->array_dims != b->array_dims) return false;
    
    for (int i = 0; i < a->array_dims; i++) {
        if (a->array_sizes[i] != b->array_sizes[i]) return false;
    }
    
    return true;
}

bool type_compatible(type_info_t *a, type_info_t *b) {
    if (type_equal(a, b)) return true;
    
    /* Numeric promotions */
    if ((a->base_type == TYPE_INT || a->base_type == TYPE_CHAR || a->base_type == TYPE_SHORT) &&
        (b->base_type == TYPE_INT || b->base_type == TYPE_CHAR || b->base_type == TYPE_SHORT)) {
        return true;
    }
    
    /* Pointer to void* compatibility */
    if (a->base_type == TYPE_POINTER && b->base_type == TYPE_POINTER) {
        if (a->pointed_type->base_type == TYPE_VOID || b->pointed_type->base_type == TYPE_VOID) {
            return true;
        }
    }
    
    return false;
}

int type_size(type_info_t *type) {
    if (type == NULL) return 0;
    
    int base_size = 0;
    switch (type->base_type) {
        case TYPE_VOID:     base_size = 0; break;
        case TYPE_CHAR:     base_size = 1; break;
        case TYPE_SHORT:    base_size = 2; break;
        case TYPE_INT:      base_size = 4; break;
        case TYPE_LONG:     base_size = 8; break;
        case TYPE_FLOAT:    base_size = 4; break;
        case TYPE_DOUBLE:   base_size = 8; break;
        case TYPE_POINTER:  base_size = 4; break;
        case TYPE_PID:      base_size = 4; break;
        case TYPE_SEMAPHORE: base_size = 4; break;
        default:            base_size = 4; break;
    }
    
    for (int i = 0; i < type->array_dims; i++) {
        base_size *= type->array_sizes[i];
    }
    
    return base_size;
}

const char *type_to_string(type_info_t *type) {
    static char buffer[256];
    
    if (type == NULL) {
        return "unknown";
    }
    
    switch (type->base_type) {
        case TYPE_VOID:     return "void";
        case TYPE_CHAR:     return "char";
        case TYPE_SHORT:    return "short";
        case TYPE_INT:      return "int";
        case TYPE_LONG:     return "long";
        case TYPE_FLOAT:    return "float";
        case TYPE_DOUBLE:   return "double";
        case TYPE_POINTER:
            snprintf(buffer, sizeof(buffer), "%s*", 
                     type->pointed_type ? type_to_string(type->pointed_type) : "void");
            return buffer;
        case TYPE_PROCESS:  return "process";
        case TYPE_SEMAPHORE: return "semaphore";
        case TYPE_PID:      return "pid";
        default:            return "unknown";
    }
}

/*  Expression Parsing  */

ast_node_t *parse_primary_expression(parser_t *parser) {
    ast_node_t *node;
    
    /* Number literal */
    if (match(parser, TOKEN_NUMBER)) {
        node = ast_create_node(NODE_NUMBER);
        node->value.int_val = parser->previous.literal.int_val;
        strcpy(node->name, parser->previous.value);
        node->data_type = type_create(TYPE_INT);
        return node;
    }
    
    if (match(parser, TOKEN_FLOAT)) {
        node = ast_create_node(NODE_FLOAT);
        node->value.float_val = parser->previous.literal.float_val;
        strcpy(node->name, parser->previous.value);
        node->data_type = type_create(TYPE_FLOAT);
        return node;
    }
    
    /* String literal */
    if (match(parser, TOKEN_STRING)) {
        node = ast_create_node(NODE_STRING);
        node->value.string_val = strdup(parser->previous.value);
        node->data_type = type_create_pointer(type_create(TYPE_CHAR));
        return node;
    }
    
    if (match(parser, TOKEN_CHAR)) {
        node = ast_create_node(NODE_CHAR);
        node->value.char_val = parser->previous.literal.char_val;
        node->data_type = type_create(TYPE_CHAR);
        return node;
    }
    
    /* Identifier */
    if (match(parser, TOKEN_IDENTIFIER)) {
        node = ast_create_node(NODE_IDENTIFIER);
        strcpy(node->name, parser->previous.value);
        node->is_lvalue = true;
        return node;
    }
    
    if (match(parser, TOKEN_TRUE) || match(parser, TOKEN_FALSE)) {
        node = ast_create_node(NODE_NUMBER);
        node->value.int_val = (parser->previous.type == TOKEN_TRUE) ? 1 : 0;
        node->data_type = type_create(TYPE_INT);
        return node;
    }
    
    /* null */
    if (match(parser, TOKEN_NULL_LITERAL)) {
        node = ast_create_node(NODE_NUMBER);
        node->value.int_val = 0;
        node->data_type = type_create(TYPE_POINTER);
        return node;
    }
    
    if (match(parser, TOKEN_LPAREN)) {
        node = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
        return node;
    }
    
    parser_error(parser, "Expected expression");
    return NULL;
}

ast_node_t *parse_postfix_expression(parser_t *parser) {
    ast_node_t *node = parse_primary_expression(parser);
    
    while (true) {
        /* Function call */
        if (match(parser, TOKEN_LPAREN)) {
            ast_node_t *call = ast_create_node(NODE_CALL);
            call->left = node;
            
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    ast_node_t *arg = parse_assignment_expression(parser);
                    ast_add_child(call, arg);
                } while (match(parser, TOKEN_COMMA));
            }
            
            consume(parser, TOKEN_RPAREN, "Expected ')' after arguments");
            node = call;
        }
        /* Array access */
        else if (match(parser, TOKEN_LBRACKET)) {
            ast_node_t *access = ast_create_node(NODE_ARRAY_ACCESS);
            access->left = node;
            access->right = parse_expression(parser);
            consume(parser, TOKEN_RBRACKET, "Expected ']' after array index");
            node = access;
            node->is_lvalue = true;
        }
        else if (match(parser, TOKEN_DOT)) {
            ast_node_t *member = ast_create_node(NODE_MEMBER_ACCESS);
            member->left = node;
            consume(parser, TOKEN_IDENTIFIER, "Expected member name");
            ast_node_t *field = ast_create_node(NODE_IDENTIFIER);
            strcpy(field->name, parser->previous.value);
            member->right = field;
            node = member;
            node->is_lvalue = true;
        }
        /* Pointer member access */
        else if (match(parser, TOKEN_ARROW)) {
            ast_node_t *member = ast_create_node(NODE_PTR_MEMBER);
            member->left = node;
            consume(parser, TOKEN_IDENTIFIER, "Expected member name");
            ast_node_t *field = ast_create_node(NODE_IDENTIFIER);
            strcpy(field->name, parser->previous.value);
            member->right = field;
            node = member;
            node->is_lvalue = true;
        }
        else if (match(parser, TOKEN_INCREMENT)) {
            ast_node_t *inc = ast_create_node(NODE_POST_INC);
            inc->left = node;
            node = inc;
        }
        /* Post-decrement */
        else if (match(parser, TOKEN_DECREMENT)) {
            ast_node_t *dec = ast_create_node(NODE_POST_DEC);
            dec->left = node;
            node = dec;
        }
        else {
            break;
        }
    }
    
    return node;
}

ast_node_t *parse_unary_expression(parser_t *parser) {
    if (match(parser, TOKEN_INCREMENT)) {
        ast_node_t *node = ast_create_node(NODE_PRE_INC);
        node->left = parse_unary_expression(parser);
        return node;
    }
    
    /* Pre-decrement */
    if (match(parser, TOKEN_DECREMENT)) {
        ast_node_t *node = ast_create_node(NODE_PRE_DEC);
        node->left = parse_unary_expression(parser);
        return node;
    }
    
    if (match(parser, TOKEN_PLUS)) {
        return parse_unary_expression(parser);
    }
    
    if (match(parser, TOKEN_MINUS)) {
        ast_node_t *node = ast_create_node(NODE_UNARY_OP);
        strcpy(node->op, "-");
        node->left = parse_unary_expression(parser);
        return node;
    }
    
    /* Logical NOT */
    if (match(parser, TOKEN_NOT)) {
        ast_node_t *node = ast_create_node(NODE_UNARY_OP);
        strcpy(node->op, "!");
        node->left = parse_unary_expression(parser);
        return node;
    }
    
    if (match(parser, TOKEN_BIT_NOT)) {
        ast_node_t *node = ast_create_node(NODE_UNARY_OP);
        strcpy(node->op, "~");
        node->left = parse_unary_expression(parser);
        return node;
    }
    
    /* Address-of */
    if (match(parser, TOKEN_BIT_AND)) {
        ast_node_t *node = ast_create_node(NODE_ADDRESS_OF);
        node->left = parse_unary_expression(parser);
        return node;
    }
    
    if (match(parser, TOKEN_MULTIPLY)) {
        ast_node_t *node = ast_create_node(NODE_DEREFERENCE);
        node->left = parse_unary_expression(parser);
        node->is_lvalue = true;
        return node;
    }
    
    if (match(parser, TOKEN_SIZEOF)) {
        ast_node_t *node = ast_create_node(NODE_SIZEOF);
        if (match(parser, TOKEN_LPAREN)) {
            node->left = parse_expression(parser);
            consume(parser, TOKEN_RPAREN, "Expected ')' after sizeof expression");
        } else {
            node->left = parse_unary_expression(parser);
        }
        node->data_type = type_create(TYPE_INT);
        return node;
    }
    
    return parse_postfix_expression(parser);
}

ast_node_t *parse_multiplicative_expression(parser_t *parser) {
    ast_node_t *left = parse_unary_expression(parser);
    
    while (match(parser, TOKEN_MULTIPLY) || match(parser, TOKEN_DIVIDE) || match(parser, TOKEN_MODULO)) {
        ast_node_t *node = ast_create_node(NODE_BINARY_OP);
        strcpy(node->op, parser->previous.value);
        node->left = left;
        node->right = parse_unary_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_additive_expression(parser_t *parser) {
    ast_node_t *left = parse_multiplicative_expression(parser);
    
    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        ast_node_t *node = ast_create_node(NODE_BINARY_OP);
        strcpy(node->op, parser->previous.value);
        node->left = left;
        node->right = parse_multiplicative_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_shift_expression(parser_t *parser) {
    ast_node_t *left = parse_additive_expression(parser);
    
    while (match(parser, TOKEN_LSHIFT) || match(parser, TOKEN_RSHIFT)) {
        ast_node_t *node = ast_create_node(NODE_BINARY_OP);
        strcpy(node->op, parser->previous.value);
        node->left = left;
        node->right = parse_additive_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_relational_expression(parser_t *parser) {
    ast_node_t *left = parse_shift_expression(parser);
    
    while (match(parser, TOKEN_LT) || match(parser, TOKEN_GT) || 
           match(parser, TOKEN_LE) || match(parser, TOKEN_GE)) {
        ast_node_t *node = ast_create_node(NODE_BINARY_OP);
        strcpy(node->op, parser->previous.value);
        node->left = left;
        node->right = parse_shift_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_equality_expression(parser_t *parser) {
    ast_node_t *left = parse_relational_expression(parser);
    
    while (match(parser, TOKEN_EQ) || match(parser, TOKEN_NE)) {
        ast_node_t *node = ast_create_node(NODE_BINARY_OP);
        strcpy(node->op, parser->previous.value);
        node->left = left;
        node->right = parse_relational_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_bitwise_and_expression(parser_t *parser) {
    ast_node_t *left = parse_equality_expression(parser);
    
    while (match(parser, TOKEN_BIT_AND)) {
        ast_node_t *node = ast_create_node(NODE_BINARY_OP);
        strcpy(node->op, "&");
        node->left = left;
        node->right = parse_equality_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_bitwise_xor_expression(parser_t *parser) {
    ast_node_t *left = parse_bitwise_and_expression(parser);
    
    while (match(parser, TOKEN_BIT_XOR)) {
        ast_node_t *node = ast_create_node(NODE_BINARY_OP);
        strcpy(node->op, "^");
        node->left = left;
        node->right = parse_bitwise_and_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_bitwise_or_expression(parser_t *parser) {
    ast_node_t *left = parse_bitwise_xor_expression(parser);
    
    while (match(parser, TOKEN_BIT_OR)) {
        ast_node_t *node = ast_create_node(NODE_BINARY_OP);
        strcpy(node->op, "|");
        node->left = left;
        node->right = parse_bitwise_xor_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_logical_and_expression(parser_t *parser) {
    ast_node_t *left = parse_bitwise_or_expression(parser);
    
    while (match(parser, TOKEN_AND)) {
        ast_node_t *node = ast_create_node(NODE_BINARY_OP);
        strcpy(node->op, "&&");
        node->left = left;
        node->right = parse_bitwise_or_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_logical_or_expression(parser_t *parser) {
    ast_node_t *left = parse_logical_and_expression(parser);
    
    while (match(parser, TOKEN_OR)) {
        ast_node_t *node = ast_create_node(NODE_BINARY_OP);
        strcpy(node->op, "||");
        node->left = left;
        node->right = parse_logical_and_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_conditional_expression(parser_t *parser) {
    ast_node_t *left = parse_logical_or_expression(parser);
    
    if (match(parser, TOKEN_QUESTION)) {
        ast_node_t *node = ast_create_node(NODE_TERNARY);
        node->left = left;
        node->right = parse_expression(parser);
        consume(parser, TOKEN_COLON, "Expected ':' in ternary expression");
        node->extra = parse_conditional_expression(parser);
        return node;
    }
    
    return left;
}

ast_node_t *parse_assignment_expression(parser_t *parser) {
    ast_node_t *left = parse_conditional_expression(parser);
    
    if (token_is_assignment_op(parser->current.type)) {
        token_type_t op_type = parser->current.type;
        advance(parser);
        
        ast_node_t *node;
        if (op_type == TOKEN_ASSIGN) {
            node = ast_create_node(NODE_ASSIGN);
        } else {
            node = ast_create_node(NODE_COMPOUND_ASSIGN);
            strcpy(node->op, parser->previous.value);
        }
        
        node->left = left;
        node->right = parse_assignment_expression(parser);
        return node;
    }
    
    return left;
}

ast_node_t *parse_expression(parser_t *parser) {
    ast_node_t *left = parse_assignment_expression(parser);
    
    /* Comma operator */
    while (match(parser, TOKEN_COMMA)) {
        ast_node_t *node = ast_create_node(NODE_COMMA);
        node->left = left;
        node->right = parse_assignment_expression(parser);
        left = node;
    }
    
    return left;
}

ast_node_t *parse_block(parser_t *parser) {
    ast_node_t *block = ast_create_node(NODE_BLOCK);
    
    consume(parser, TOKEN_LBRACE, "Expected '{'");
    
    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        ast_node_t *stmt = parse_statement(parser);
        if (stmt != NULL) {
            ast_add_child(block, stmt);
        }
    }
    
    consume(parser, TOKEN_RBRACE, "Expected '}'");
    
    return block;
}

ast_node_t *parse_if_statement(parser_t *parser) {
    ast_node_t *node = ast_create_node(NODE_IF);
    
    consume(parser, TOKEN_LPAREN, "Expected '(' after 'if'");
    node->left = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expected ')' after condition");
    
    node->right = parse_statement(parser);
    
    if (match(parser, TOKEN_ELSE)) {
        node->extra = parse_statement(parser);
    }
    
    return node;
}

ast_node_t *parse_while_statement(parser_t *parser) {
    ast_node_t *node = ast_create_node(NODE_WHILE);
    
    consume(parser, TOKEN_LPAREN, "Expected '(' after 'while'");
    node->left = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expected ')' after condition");
    
    node->right = parse_statement(parser);
    
    return node;
}

ast_node_t *parse_do_while_statement(parser_t *parser) {
    ast_node_t *node = ast_create_node(NODE_DO_WHILE);
    
    node->left = parse_statement(parser);
    
    consume(parser, TOKEN_WHILE, "Expected 'while' after do body");
    consume(parser, TOKEN_LPAREN, "Expected '(' after 'while'");
    node->right = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expected ')' after condition");
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after do-while");
    
    return node;
}

ast_node_t *parse_for_statement(parser_t *parser) {
    ast_node_t *node = ast_create_node(NODE_FOR);
    
    consume(parser, TOKEN_LPAREN, "Expected '(' after 'for'");
    
    /* Init */
    if (match(parser, TOKEN_SEMICOLON)) {
        node->left = NULL;
    } else {
        node->left = parse_expression(parser);
        consume(parser, TOKEN_SEMICOLON, "Expected ';' after for init");
    }
    
    /* Condition */
    if (match(parser, TOKEN_SEMICOLON)) {
        node->right = NULL;
    } else {
        node->right = parse_expression(parser);
        consume(parser, TOKEN_SEMICOLON, "Expected ';' after for condition");
    }
    
    /* Increment */
    if (check(parser, TOKEN_RPAREN)) {
        node->extra = NULL;
    } else {
        node->extra = parse_expression(parser);
    }
    
    consume(parser, TOKEN_RPAREN, "Expected ')' after for clauses");
    
    /* Body */
    ast_node_t *body = parse_statement(parser);
    ast_add_child(node, body);
    
    return node;
}

ast_node_t *parse_return_statement(parser_t *parser) {
    ast_node_t *node = ast_create_node(NODE_RETURN);
    
    if (!check(parser, TOKEN_SEMICOLON)) {
        node->left = parse_expression(parser);
    }
    
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after return");
    
    return node;
}

ast_node_t *parse_break_statement(parser_t *parser) {
    ast_node_t *node = ast_create_node(NODE_BREAK);
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after break");
    return node;
}

ast_node_t *parse_continue_statement(parser_t *parser) {
    ast_node_t *node = ast_create_node(NODE_CONTINUE);
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after continue");
    return node;
}

ast_node_t *parse_statement(parser_t *parser) {
    if (check(parser, TOKEN_LBRACE)) {
        return parse_block(parser);
    }
    
    if (match(parser, TOKEN_IF)) {
        return parse_if_statement(parser);
    }
    
    /* While loop */
    if (match(parser, TOKEN_WHILE)) {
        return parse_while_statement(parser);
    }
    
    if (match(parser, TOKEN_DO)) {
        return parse_do_while_statement(parser);
    }
    
    if (match(parser, TOKEN_FOR)) {
        return parse_for_statement(parser);
    }
    
    if (match(parser, TOKEN_RETURN)) {
        return parse_return_statement(parser);
    }
    
    /* Break */
    if (match(parser, TOKEN_BREAK)) {
        return parse_break_statement(parser);
    }
    
    /* Continue */
    if (match(parser, TOKEN_CONTINUE)) {
        return parse_continue_statement(parser);
    }
    
    ast_node_t *node = ast_create_node(NODE_EXPR_STMT);
    node->left = parse_expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after expression");
    
    return node;
}

/*  Declaration Parsing  */

type_info_t *parse_type_specifier(parser_t *parser) {
    type_info_t *type = NULL;
    
    if (match(parser, TOKEN_VOID)) {
        type = type_create(TYPE_VOID);
    } else if (match(parser, TOKEN_CHAR_TYPE)) {
        type = type_create(TYPE_CHAR);
    } else if (match(parser, TOKEN_SHORT)) {
        type = type_create(TYPE_SHORT);
    } else if (match(parser, TOKEN_INT)) {
        type = type_create(TYPE_INT);
    } else if (match(parser, TOKEN_LONG)) {
        type = type_create(TYPE_LONG);
    } else if (match(parser, TOKEN_FLOAT_TYPE)) {
        type = type_create(TYPE_FLOAT);
    } else if (match(parser, TOKEN_DOUBLE)) {
        type = type_create(TYPE_DOUBLE);
    } else if (match(parser, TOKEN_PROCESS)) {
        type = type_create(TYPE_PROCESS);
    } else if (match(parser, TOKEN_SEMAPHORE)) {
        type = type_create(TYPE_SEMAPHORE);
    } else {
        parser_error(parser, "Expected type specifier");
        return type_create(TYPE_INT);
    }
    
    if (match(parser, TOKEN_UNSIGNED)) {
        type->qualifiers |= QUAL_UNSIGNED;
    } else if (match(parser, TOKEN_SIGNED)) {
        type->qualifiers |= QUAL_SIGNED;
    }
    
    if (match(parser, TOKEN_CONST)) {
        type->qualifiers |= QUAL_CONST;
    }
    
    if (match(parser, TOKEN_VOLATILE)) {
        type->qualifiers |= QUAL_VOLATILE;
    }
    
    return type;
}

ast_node_t *parse_variable_declaration(parser_t *parser, type_info_t *type) {
    ast_node_t *node = ast_create_node(NODE_VAR_DECL);
    node->data_type = type;
    
    consume(parser, TOKEN_IDENTIFIER, "Expected variable name");
    strcpy(node->name, parser->previous.value);
    
    while (match(parser, TOKEN_LBRACKET)) {
        if (check(parser, TOKEN_NUMBER)) {
            int size = (int)parser->current.literal.int_val;
            advance(parser);
            node->data_type = type_create_array(node->data_type, size);
        } else {
            node->data_type = type_create_array(node->data_type, 0);  /* Unknown size */
        }
        consume(parser, TOKEN_RBRACKET, "Expected ']'");
    }
    
    /* Initializer */
    if (match(parser, TOKEN_ASSIGN)) {
        node->left = parse_assignment_expression(parser);
    }
    
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after declaration");
    
    return node;
}

ast_node_t *parse_function(parser_t *parser, type_info_t *return_type, const char *name) {
    ast_node_t *node = ast_create_node(NODE_FUNCTION);
    node->data_type = return_type;
    strcpy(node->name, name);
    
    consume(parser, TOKEN_LPAREN, "Expected '(' after function name");
    
    if (!check(parser, TOKEN_RPAREN)) {
        do {
            type_info_t *param_type = parse_type_specifier(parser);
            ast_node_t *param = ast_create_node(NODE_PARAM);
            param->data_type = param_type;
            
            if (check(parser, TOKEN_IDENTIFIER)) {
                strcpy(param->name, parser->current.value);
                advance(parser);
            }
            
            ast_add_child(node, param);
        } while (match(parser, TOKEN_COMMA));
    }
    
    consume(parser, TOKEN_RPAREN, "Expected ')' after parameters");
    
    /* Function body */
    if (check(parser, TOKEN_LBRACE)) {
        node->left = parse_block(parser);
    } else {
        consume(parser, TOKEN_SEMICOLON, "Expected ';' or function body");
    }
    
    return node;
}

ast_node_t *parse_declaration(parser_t *parser) {
    if (match(parser, TOKEN_STATIC)) {
    } else if (match(parser, TOKEN_EXTERN)) {
    }
    
    type_info_t *type = parse_type_specifier(parser);
    
    while (match(parser, TOKEN_MULTIPLY)) {
        type = type_create_pointer(type);
    }
    
    consume(parser, TOKEN_IDENTIFIER, "Expected identifier");
    char name[MAX_TOKEN_LEN];
    strcpy(name, parser->previous.value);
    
    if (check(parser, TOKEN_LPAREN)) {
        return parse_function(parser, type, name);
    } else {
        parser->current = parser->previous;
        advance(parser);
        return parse_variable_declaration(parser, type);
    }
}

ast_node_t *parser_parse_program(parser_t *parser) {
    ast_node_t *program = ast_create_node(NODE_PROGRAM);
    
    while (!check(parser, TOKEN_EOF)) {
        if (parser->had_error) {
            parser_synchronize(parser);
            continue;
        }
        
        ast_node_t *decl = parse_declaration(parser);
        if (decl != NULL) {
            ast_add_child(program, decl);
        }
    }
    
    return program;
}

ast_node_t *parser_parse(parser_t *parser) {
    return parser_parse_program(parser);
}

void parser_error(parser_t *parser, const char *message) {
    parser_error_at(parser, &parser->current, message);
}

void parser_error_at(parser_t *parser, token_t *token, const char *message) {
    if (parser->panic_mode) return;
    
    parser->panic_mode = true;
    parser->had_error = true;
    
    snprintf(parser->error_msg, sizeof(parser->error_msg),
             "%s:%d:%d: error: %s at '%s'",
             parser->filename, token->line, token->column,
             message, token->value);
}

bool parser_had_error(parser_t *parser) {
    return parser->had_error;
}

void parser_synchronize(parser_t *parser) {
    parser->panic_mode = false;
    
    while (parser->current.type != TOKEN_EOF) {
        if (parser->previous.type == TOKEN_SEMICOLON) return;
        
        switch (parser->current.type) {
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_FOR:
            case TOKEN_RETURN:
            case TOKEN_INT:
            case TOKEN_VOID:
            case TOKEN_CHAR_TYPE:
            case TOKEN_FLOAT_TYPE:
                return;
            default:
                ;  /* Do nothing */
        }
        
        advance(parser);
    }
}

/*  Simple API  */

ast_node_t *parse_program(char *source) {
    lexer_t lexer;
    lexer_init_ex(&lexer, source, "<stdin>");
    parser_init(&global_parser, &lexer);
    return parser_parse(&global_parser);
}

void ast_print(ast_node_t *node, int indent) {
    if (node == NULL) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_NUMBER:
            printf("NUMBER: %lld\n", node->value.int_val);
            break;
        case NODE_IDENTIFIER:
            printf("IDENTIFIER: %s\n", node->name);
            break;
        case NODE_BINARY_OP:
            printf("BINARY_OP: %s\n", node->op);
            ast_print(node->left, indent + 1);
            ast_print(node->right, indent + 1);
            break;
        case NODE_ASSIGN:
            printf("ASSIGN\n");
            ast_print(node->left, indent + 1);
            ast_print(node->right, indent + 1);
            break;
        case NODE_FUNCTION:
            printf("FUNCTION: %s\n", node->name);
            for (int i = 0; i < node->child_count; i++) {
                ast_print(node->children[i], indent + 1);
            }
            ast_print(node->left, indent + 1);
            break;
        case NODE_BLOCK:
            printf("BLOCK\n");
            for (int i = 0; i < node->child_count; i++) {
                ast_print(node->children[i], indent + 1);
            }
            break;
        default:
            printf("NODE (type %d)\n", node->type);
            break;
    }
}

