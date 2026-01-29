#include "symbol_table.h"
#include "../include/memory.h"
#include <string.h>
#include <stdio.h>

#define HASH_TABLE_SIZE 128

static unsigned int hash(const char *str) {
    unsigned int hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return hash % HASH_TABLE_SIZE;
}

static scope_t *scope_create(scope_t *parent, int level) {
    scope_t *scope = (scope_t *)getmem(sizeof(scope_t));
    if (scope == NULL) return NULL;
    
    scope->level = level;
    scope->symbols = (symbol_t **)getmem(sizeof(symbol_t *) * HASH_TABLE_SIZE);
    if (scope->symbols == NULL) {
        freemem(scope, sizeof(scope_t));
        return NULL;
    }
    
    memset(scope->symbols, 0, sizeof(symbol_t *) * HASH_TABLE_SIZE);
    scope->symbol_count = 0;
    scope->next_offset = 0;
    scope->parent = parent;
    
    return scope;
}

static void scope_destroy(scope_t *scope) {
    if (scope == NULL) return;
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        symbol_t *sym = scope->symbols[i];
        while (sym != NULL) {
            symbol_t *next = sym->next;
            if (sym->type != NULL) {
                type_free(sym->type);
            }
            freemem(sym, sizeof(symbol_t));
            sym = next;
        }
    }
    
    freemem(scope->symbols, sizeof(symbol_t *) * HASH_TABLE_SIZE);
    freemem(scope, sizeof(scope_t));
}

symbol_table_t *symtab_create(void) {
    symbol_table_t *table = (symbol_table_t *)getmem(sizeof(symbol_table_t));
    if (table == NULL) return NULL;
    
    table->scope_level = 0;
    table->global_scope = scope_create(NULL, 0);
    table->current_scope = table->global_scope;
    table->had_error = false;
    table->error_msg[0] = '\0';
    
    return table;
}

void symtab_destroy(symbol_table_t *table) {
    if (table == NULL) return;
    
    scope_t *scope = table->current_scope;
    while (scope != NULL) {
        scope_t *parent = scope->parent;
        scope_destroy(scope);
        scope = parent;
    }
    
    freemem(table, sizeof(symbol_table_t));
}

void symtab_enter_scope(symbol_table_t *table) {
    table->scope_level++;
    scope_t *new_scope = scope_create(table->current_scope, table->scope_level);
    table->current_scope = new_scope;
}

void symtab_exit_scope(symbol_table_t *table) {
    if (table->current_scope == table->global_scope) {
        return;
    }
    
    scope_t *old_scope = table->current_scope;
    table->current_scope = old_scope->parent;
    table->scope_level--;
    scope_destroy(old_scope);
}

symbol_t *symtab_insert(symbol_table_t *table, const char *name, symbol_kind_t kind, type_info_t *type) {
    if (table == NULL || name == NULL) return NULL;
    
    if (symtab_exists_current_scope(table, name)) {
        snprintf(table->error_msg, sizeof(table->error_msg),
                 "Symbol '%s' already declared in current scope", name);
        table->had_error = true;
        return NULL;
    }
    
    symbol_t *symbol = (symbol_t *)getmem(sizeof(symbol_t));
    if (symbol == NULL) return NULL;
    
    strncpy(symbol->name, name, MAX_TOKEN_LEN - 1);
    symbol->name[MAX_TOKEN_LEN - 1] = '\0';
    symbol->kind = kind;
    symbol->type = type;
    symbol->scope_level = table->scope_level;
    symbol->offset = table->current_scope->next_offset;
    symbol->is_initialized = false;
    symbol->is_used = false;
    symbol->declaration = NULL;
    
    if (kind == SYM_VARIABLE || kind == SYM_PARAMETER) {
        table->current_scope->next_offset += type_size(type);
    }
    
    unsigned int h = hash(name);
    symbol->next = table->current_scope->symbols[h];
    table->current_scope->symbols[h] = symbol;
    table->current_scope->symbol_count++;
    
    return symbol;
}

symbol_t *symtab_lookup(symbol_table_t *table, const char *name) {
    if (table == NULL || name == NULL) return NULL;
    
    unsigned int h = hash(name);
    scope_t *scope = table->current_scope;
    
    while (scope != NULL) {
        symbol_t *sym = scope->symbols[h];
        while (sym != NULL) {
            if (strcmp(sym->name, name) == 0) {
                return sym;
            }
            sym = sym->next;
        }
        scope = scope->parent;
    }
    
    return NULL;
}

symbol_t *symtab_lookup_current_scope(symbol_table_t *table, const char *name) {
    if (table == NULL || name == NULL) return NULL;
    
    unsigned int h = hash(name);
    symbol_t *sym = table->current_scope->symbols[h];
    
    while (sym != NULL) {
        if (strcmp(sym->name, name) == 0) {
            return sym;
        }
        sym = sym->next;
    }
    
    return NULL;
}

bool symtab_exists_current_scope(symbol_table_t *table, const char *name) {
    return symtab_lookup_current_scope(table, name) != NULL;
}

const char *symbol_kind_to_string(symbol_kind_t kind) {
    switch (kind) {
        case SYM_VARIABLE:      return "variable";
        case SYM_PARAMETER:     return "parameter";
        case SYM_FUNCTION:      return "function";
        case SYM_PROCESS:       return "process";
        case SYM_SEMAPHORE:     return "semaphore";
        case SYM_STRUCT:        return "struct";
        case SYM_UNION:         return "union";
        case SYM_ENUM:          return "enum";
        case SYM_TYPEDEF:       return "typedef";
        case SYM_LABEL:         return "label";
        default:                return "unknown";
    }
}

void symtab_print(symbol_table_t *table) {
    if (table == NULL) return;
    
    scope_t *scope = table->current_scope;
    int level = 0;
    
    while (scope != NULL) {
        printf("Scope level %d:\n", scope->level);
        
        for (int i = 0; i < HASH_TABLE_SIZE; i++) {
            symbol_t *sym = scope->symbols[i];
            while (sym != NULL) {
                printf("  %s: %s (%s) offset=%d\n",
                       sym->name,
                       symbol_kind_to_string(sym->kind),
                       type_to_string(sym->type),
                       sym->offset);
                sym = sym->next;
            }
        }
        
        scope = scope->parent;
    }
}

void symtab_error(symbol_table_t *table, const char *message) {
    table->had_error = true;
    strncpy(table->error_msg, message, sizeof(table->error_msg) - 1);
    table->error_msg[sizeof(table->error_msg) - 1] = '\0';
}

bool symtab_had_error(symbol_table_t *table) {
    return table->had_error;
}
