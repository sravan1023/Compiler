#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include "parser.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SYM_VARIABLE,
    SYM_PARAMETER,
    SYM_FUNCTION,
    SYM_PROCESS,
    SYM_SEMAPHORE,
    SYM_STRUCT,
    SYM_UNION,
    SYM_ENUM,
    SYM_TYPEDEF,
    SYM_LABEL
} symbol_kind_t;

typedef struct symbol {
    char name[MAX_TOKEN_LEN];
    symbol_kind_t kind;
    type_info_t *type;
    int scope_level;
    int offset;
    bool is_initialized;
    bool is_used;
    ast_node_t *declaration;
    struct symbol *next;
} symbol_t;

typedef struct scope {
    int level;
    symbol_t **symbols;
    int symbol_count;
    int next_offset;
    struct scope *parent;
} scope_t;

typedef struct {
    scope_t *current_scope;
    scope_t *global_scope;
    int scope_level;
    bool had_error;
    char error_msg[512];
} symbol_table_t;

symbol_table_t *symtab_create(void);
void symtab_destroy(symbol_table_t *table);
void symtab_enter_scope(symbol_table_t *table);
void symtab_exit_scope(symbol_table_t *table);

symbol_t *symtab_insert(symbol_table_t *table, const char *name, symbol_kind_t kind, type_info_t *type);
symbol_t *symtab_lookup(symbol_table_t *table, const char *name);
symbol_t *symtab_lookup_current_scope(symbol_table_t *table, const char *name);
bool symtab_exists_current_scope(symbol_table_t *table, const char *name);

const char *symbol_kind_to_string(symbol_kind_t kind);
void symtab_print(symbol_table_t *table);

void symtab_error(symbol_table_t *table, const char *message);
bool symtab_had_error(symbol_table_t *table);

#endif
