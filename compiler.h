#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "lexer.h"
#include "parser.h"
#include "symbol_table.h"
#include "codegen.h"
#include <stdbool.h>

/* Compilation options */
typedef struct {
    bool dump_tokens;           /* Print all tokens */
    bool dump_ast;              /* Print AST */
    bool dump_symbols;          /* Print symbol table */
    bool dump_code;             /* Print generated code */
    bool optimize;              /* Enable optimizations */
    int warning_level;          /* Warning level (0-3) */
    const char *output_file;    /* Output filename */
} compiler_options_t;

/* Compiler state */
typedef struct {
    const char *source;         /* Source code */
    const char *filename;       /* Source filename */
    compiler_options_t options; /* Compilation options */
    
    /* Compilation phases */
    lexer_t *lexer;             /* Lexical analyzer */
    parser_t *parser;           /* Parser */
    symbol_table_t *symtab;     /* Symbol table */
    codegen_t *codegen;         /* Code generator */
    
    /* Results */
    ast_node_t *ast;            /* Abstract syntax tree */
    code_buffer_t *code;        /* Generated code */
    
    /* Error tracking */
    bool had_error;             /* Any errors occurred */
    bool had_warning;           /* Any warnings occurred */
    int error_count;            /* Number of errors */
    int warning_count;          /* Number of warnings */
    char error_msg[1024];       /* Last error message */
} compiler_t;

/* Compiler lifecycle */
compiler_t *compiler_create(const char *source, const char *filename);
void compiler_destroy(compiler_t *compiler);
void compiler_set_options(compiler_t *compiler, compiler_options_t *options);

/* Compilation pipeline */
bool compiler_compile(compiler_t *compiler);
bool compiler_lex(compiler_t *compiler);
bool compiler_parse(compiler_t *compiler);
bool compiler_analyze(compiler_t *compiler);
bool compiler_generate(compiler_t *compiler);

/* Output */
bool compiler_write_output(compiler_t *compiler, const char *filename);
void compiler_print_errors(compiler_t *compiler);
void compiler_print_warnings(compiler_t *compiler);
void compiler_print_stats(compiler_t *compiler);

/* Simple API */
void *compile(char *source);
bool compile_file(const char *input_file, const char *output_file);
void free_compiled(void *code);

/* Error handling */
void compiler_error(compiler_t *compiler, const char *message);
void compiler_warning(compiler_t *compiler, const char *message);
bool compiler_had_error(compiler_t *compiler);
bool compiler_had_warning(compiler_t *compiler);

/* Utility functions */
const char *compiler_get_version(void);
void compiler_print_help(void);

#endif


