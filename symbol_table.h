#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include "parser.h"
#include <stdint.h>
#include <stdbool.h>

/* Symbol kinds */
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

/* Symbol structure */
typedef struct symbol {
    char name[MAX_TOKEN_LEN];       /* Symbol name */
    symbol_kind_t kind;             /* Symbol kind */
    type_info_t *type;              /* Type information */
    int scope_level;                /* Scope nesting level */
    int offset;                     /* Stack offset or address */
    bool is_initialized;            /* Has been initialized */
    bool is_used;                   /* Has been referenced */
    ast_node_t *declaration;        /* Declaration AST node */
    struct symbol *next;            /* Next in hash chain */
} symbol_t;

/* Scope structure */
typedef struct scope {
    int level;                      /* Nesting level */
    symbol_t **symbols;             /* Hash table of symbols */
    int symbol_count;               /* Number of symbols */
    int next_offset;                /* Next available stack offset */
    struct scope *parent;           /* Parent scope */
} scope_t;

/* Symbol table structure */
typedef struct {
    scope_t *current_scope;         /* Current active scope */
    scope_t *global_scope;          /* Global scope */
    int scope_level;                /* Current scope level */
    bool had_error;                 /* Error flag */
    char error_msg[512];            /* Error message */
} symbol_table_t;

/* Symbol table functions */
symbol_table_t *symtab_create(void);
void symtab_destroy(symbol_table_t *table);
void symtab_enter_scope(symbol_table_t *table);
void symtab_exit_scope(symbol_table_t *table);

/* Symbol operations */
symbol_t *symtab_insert(symbol_table_t *table, const char *name, symbol_kind_t kind, type_info_t *type);
symbol_t *symtab_lookup(symbol_table_t *table, const char *name);
symbol_t *symtab_lookup_current_scope(symbol_table_t *table, const char *name);
bool symtab_exists_current_scope(symbol_table_t *table, const char *name);

/* Utility functions */
const char *symbol_kind_to_string(symbol_kind_t kind);
void symtab_print(symbol_table_t *table);

/* Error handling */
void symtab_error(symbol_table_t *table, const char *message);
bool symtab_had_error(symbol_table_t *table);

#endif
