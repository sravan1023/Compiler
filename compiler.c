#include "compiler.h"
#include "../include/memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define COMPILER_VERSION "1.0.0"


compiler_t *compiler_create(const char *source, const char *filename) {
    compiler_t *compiler = (compiler_t *)getmem(sizeof(compiler_t));
    if (compiler == NULL) return NULL;
    
    memset(compiler, 0, sizeof(compiler_t));
    compiler->source = source;
    compiler->filename = filename ? filename : "<stdin>";
    
    /* Initialize default options */
    compiler->options.dump_tokens = false;
    compiler->options.dump_ast = false;
    compiler->options.dump_symbols = false;
    compiler->options.dump_code = false;
    compiler->options.optimize = false;
    compiler->options.warning_level = 1;
    compiler->options.output_file = "out.xc";
    
    return compiler;
}

void compiler_destroy(compiler_t *compiler) {
    if (compiler == NULL) return;
    
    if (compiler->ast != NULL) {
        ast_free(compiler->ast);
    }
    
    if (compiler->symtab != NULL) {
        symtab_destroy(compiler->symtab);
    }
    
    if (compiler->codegen != NULL) {
        codegen_destroy(compiler->codegen);
    }
    
    if (compiler->lexer != NULL) {
        freemem(compiler->lexer, sizeof(lexer_t));
    }
    
    if (compiler->parser != NULL) {
        freemem(compiler->parser, sizeof(parser_t));
    }
    
    freemem(compiler, sizeof(compiler_t));
}

/* Set compilation options */
void compiler_set_options(compiler_t *compiler, compiler_options_t *options) {
    if (compiler == NULL || options == NULL) return;
    compiler->options = *options;
}

/* Lexical analysis */
bool compiler_lex(compiler_t *compiler) {
    if (compiler == NULL) return false;
    
    compiler->lexer = (lexer_t *)getmem(sizeof(lexer_t));
    if (compiler->lexer == NULL) {
        compiler_error(compiler, "Failed to allocate lexer");
        return false;
    }
    
    lexer_init_ex(compiler->lexer, compiler->source, compiler->filename);
    
    if (compiler->options.dump_tokens) {
        printf("=== Tokens ===\n");
        token_t token;
        do {
            token = lexer_next_token_ex(compiler->lexer);
            printf("%-15s '%s' at %d:%d\n",
                   token_type_to_string(token.type),
                   token.value,
                   token.line,
                   token.column);
        } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);
        
        lexer_init_ex(compiler->lexer, compiler->source, compiler->filename);
    }
    
    if (lexer_has_error(compiler->lexer)) {
        compiler_error(compiler, lexer_get_error(compiler->lexer));
        return false;
    }
    
    return true;
}

/* Syntactic analysis */
bool compiler_parse(compiler_t *compiler) {
    if (compiler == NULL || compiler->lexer == NULL) return false;
    
    compiler->parser = (parser_t *)getmem(sizeof(parser_t));
    if (compiler->parser == NULL) {
        compiler_error(compiler, "Failed to allocate parser");
        return false;
    }
    
    parser_init(compiler->parser, compiler->lexer);
    compiler->ast = parser_parse(compiler->parser);
    
    if (compiler->ast == NULL || parser_had_error(compiler->parser)) {
        if (compiler->parser->error_msg[0] != '\0') {
            compiler_error(compiler, compiler->parser->error_msg);
        } else {
            compiler_error(compiler, "Parsing failed");
        }
        return false;
    }
    
    if (compiler->options.dump_ast) {
        printf("\n=== Abstract Syntax Tree ===\n");
        ast_print(compiler->ast, 0);
    }
    
    return true;
}

/* Semantic analysis */
bool compiler_analyze(compiler_t *compiler) {
    if (compiler == NULL || compiler->ast == NULL) return false;
    
    /* Create symbol table */
    compiler->symtab = symtab_create();
    if (compiler->symtab == NULL) {
        compiler_error(compiler, "Failed to create symbol table");
        return false;
    }
    
    /* Build symbol table from AST */
    if (compiler->ast->type == NODE_PROGRAM) {
        for (int i = 0; i < compiler->ast->child_count; i++) {
            ast_node_t *child = compiler->ast->children[i];
            
            if (child->type == NODE_FUNCTION || child->type == NODE_PROCESS) {
                /* Add function to symbol table */
                symtab_insert(compiler->symtab, child->name, 
                             child->type == NODE_PROCESS ? SYM_PROCESS : SYM_FUNCTION,
                             child->data_type);
            } else if (child->type == NODE_VAR_DECL) {
                /* Add global variable */
                symtab_insert(compiler->symtab, child->name, SYM_VARIABLE, child->data_type);
            }
        }
    }
    
    if (compiler->options.dump_symbols) {
        printf("\n Symbol Table \n");
        symtab_print(compiler->symtab);
    }
    
    if (symtab_had_error(compiler->symtab)) {
        compiler_error(compiler, "Semantic analysis failed");
        return false;
    }
    
    return true;
}

/* Code generation */
bool compiler_generate(compiler_t *compiler) {
    if (compiler == NULL || compiler->ast == NULL) return false;
    
    /* Create code generator */
    compiler->codegen = codegen_create(compiler->symtab);
    if (compiler->codegen == NULL) {
        compiler_error(compiler, "Failed to create code generator");
        return false;
    }
    
    /* Generate code */
    if (!codegen_generate(compiler->codegen, compiler->ast)) {
        if (compiler->codegen->error_msg[0] != '\0') {
            compiler_error(compiler, compiler->codegen->error_msg);
        } else {
            compiler_error(compiler, "Code generation failed");
        }
        return false;
    }
    
    if (compiler->options.dump_code) {
        printf("\n Generated Code \n");
        codegen_print(compiler->codegen);
    }
    
    return true;
}

/* Full compilation pipeline */
bool compiler_compile(compiler_t *compiler) {
    if (compiler == NULL) return false;
    
    printf("Compiling \n", compiler->filename);
    
    /* Lexical analysis */
    if (!compiler_lex(compiler)) {
        return false;
    }
    
    /* Parsing */
    if (!compiler_parse(compiler)) {
        return false;
    }
    
    /* Semantic analysis */
    if (!compiler_analyze(compiler)) {
        return false;
    }
    
    /* Code generation */
    if (!compiler_generate(compiler)) {
        return false;
    }
    
    printf("Compilation successful!\n");
    compiler_print_stats(compiler);
    
    return true;
}

/* Write output to file */
bool compiler_write_output(compiler_t *compiler, const char *filename) {
    if (compiler == NULL || compiler->codegen == NULL) return false;
    
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        compiler_error(compiler, "Failed to open output file");
        return false;
    }
    
    /* Write header */
    fprintf(file, "; Xinu OS Compiled Code\n");
    fprintf(file, "; Source: %s\n", compiler->filename);
    fprintf(file, "; Compiler Version: %s\n\n", COMPILER_VERSION);
    
    /* Write instructions */
    for (int i = 0; i < compiler->codegen->code->count; i++) {
        instruction_t *inst = &compiler->codegen->code->instructions[i];
        
        if (inst->label[0] != '\0') {
            fprintf(file, "%s:\n", inst->label);
        }
        
        fprintf(file, "  %-10s %d\n", 
                opcode_to_string(inst->opcode), 
                inst->operand);
    }
    
    fclose(file);
    printf("Output written to %s\n", filename);
    
    return true;
}

/* Print compilation statistics */
void compiler_print_stats(compiler_t *compiler) {
    if (compiler == NULL) return;
    
    printf("\n Compilation Statistics \n");
    printf("Errors:   %d\n", compiler->error_count);
    printf("Warnings: %d\n", compiler->warning_count);
    
    if (compiler->codegen != NULL && compiler->codegen->code != NULL) {
        printf("Code size: %d instructions\n", compiler->codegen->code->count);
    }
}

/* Print errors */
void compiler_print_errors(compiler_t *compiler) {
    if (compiler == NULL || !compiler->had_error) return;
    
    printf("Error: %s\n", compiler->error_msg);
}

/* Print warnings */
void compiler_print_warnings(compiler_t *compiler) {
    if (compiler == NULL || !compiler->had_warning) return;
    printf("Warning: %s\n", compiler->error_msg);
}

/* Error reporting */
void compiler_error(compiler_t *compiler, const char *message) {
    if (compiler == NULL) return;
    
    compiler->had_error = true;
    compiler->error_count++;
    strncpy(compiler->error_msg, message, sizeof(compiler->error_msg) - 1);
    compiler->error_msg[sizeof(compiler->error_msg) - 1] = '\0';
    
    fprintf(stderr, "Error: %s\n", message);
}

void compiler_warning(compiler_t *compiler, const char *message) {
    if (compiler == NULL) return;
    
    compiler->had_warning = true;
    compiler->warning_count++;
    
    if (compiler->options.warning_level > 0) {
        fprintf(stderr, "Warning: %s\n", message);
    }
}

bool compiler_had_error(compiler_t *compiler) {
    return compiler != NULL && compiler->had_error;
}

bool compiler_had_warning(compiler_t *compiler) {
    return compiler != NULL && compiler->had_warning;
}

/* Simple compile function */
void *compile(char *source) {
    compiler_t *compiler = compiler_create(source, "<stdin>");
    if (compiler == NULL) return NULL;
    
    if (!compiler_compile(compiler)) {
        compiler_destroy(compiler);
        return NULL;
    }
    
    /* Extract code buffer before destroying compiler */
    void *code = compiler->codegen->code;
    compiler->codegen->code = NULL;  /* Prevent deallocation */
    compiler_destroy(compiler);
    
    return code;
}

/* Compile file */
bool compile_file(const char *input_file, const char *output_file) {
    /* Read source file */
    FILE *file = fopen(input_file, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", input_file);
        return false;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *source = (char *)getmem(size + 1);
    if (source == NULL) {
        fclose(file);
        return false;
    }
    
    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);
    
    /* Compile */
    compiler_t *compiler = compiler_create(source, input_file);
    if (compiler == NULL) {
        freemem(source, size + 1);
        return false;
    }
    
    bool success = compiler_compile(compiler);
    
    if (success) {
        success = compiler_write_output(compiler, output_file);
    }
    
    compiler_destroy(compiler);
    freemem(source, size + 1);
    
    return success;
}

/* Free compiled code */
void free_compiled(void *code) {
    if (code != NULL) {
        code_buffer_destroy((code_buffer_t *)code);
    }
}

const char *compiler_get_version(void) {
    return COMPILER_VERSION;
}

void compiler_print_help(void) {
    printf("Xinu OS Compiler v%s\n", COMPILER_VERSION);
    printf("Usage: compiler [options] <input_file>\n\n");
    printf("Options:\n");
    printf("  -o <file>     Write output to <file>\n");
    printf("  -dump-tokens  Print all tokens\n");
    printf("  -dump-ast     Print abstract syntax tree\n");
    printf("  -dump-symbols Print symbol table\n");
    printf("  -dump-code    Print generated code\n");
    printf("  -O            Enable optimizations\n");
    printf("  -W<level>     Set warning level (0-3)\n");
    printf("  -h, --help    Print this help message\n");
    printf("  -v, --version Print compiler version\n");
}
