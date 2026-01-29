#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "parser.h"
#include "symbol_table.h"
#include <stdint.h>
#include <stdbool.h>

/* Instruction opcodes */
typedef enum {
    /* Stack operations */
    OP_PUSH,            /* Push value onto stack */
    OP_POP,             /* Pop value from stack */
    OP_DUP,             /* Duplicate top of stack */
    OP_SWAP,            /* Swap top two stack items */
    
    /* Arithmetic */
    OP_ADD,             /* Addition */
    OP_SUB,             /* Subtraction */
    OP_MUL,             /* Multiplication */
    OP_DIV,             /* Division */
    OP_MOD,             /* Modulo */
    OP_NEG,             /* Negation */
    
    /* Bitwise operations */
    OP_AND,             /* Bitwise AND */
    OP_OR,              /* Bitwise OR */
    OP_XOR,             /* Bitwise XOR */
    OP_NOT,             /* Bitwise NOT */
    OP_SHL,             /* Shift left */
    OP_SHR,             /* Shift right */
    
    /* Logical operations */
    OP_LAND,            /* Logical AND */
    OP_LOR,             /* Logical OR */
    OP_LNOT,            /* Logical NOT */
    
    /* Comparison */
    OP_EQ,              /* Equal */
    OP_NE,              /* Not equal */
    OP_LT,              /* Less than */
    OP_LE,              /* Less or equal */
    OP_GT,              /* Greater than */
    OP_GE,              /* Greater or equal */
    
    /* Memory operations */
    OP_LOAD,            /* Load from address */
    OP_STORE,           /* Store to address */
    OP_LOADL,           /* Load local variable */
    OP_STOREL,          /* Store local variable */
    OP_LOADG,           /* Load global variable */
    OP_STOREG,          /* Store global variable */
    OP_ADDR,            /* Get address of variable */
    
    /* Control flow */
    OP_JMP,             /* Unconditional jump */
    OP_JZ,              /* Jump if zero */
    OP_JNZ,             /* Jump if not zero */
    OP_CALL,            /* Function call */
    OP_RET,             /* Return from function */
    
    /* Xinu-specific */
    OP_CREATE,          /* Create process */
    OP_RESUME,          /* Resume process */
    OP_SUSPEND,         /* Suspend process */
    OP_KILL,            /* Kill process */
    OP_SLEEP,           /* Sleep */
    OP_YIELD,           /* Yield CPU */
    OP_WAIT,            /* Wait on semaphore */
    OP_SIGNAL,          /* Signal semaphore */
    OP_GETPID,          /* Get process ID */
    
    OP_NOP,             /* No operation */
    OP_HALT             /* Halt execution */
} opcode_t;

typedef struct {
    opcode_t opcode;        /* Operation code */
    int32_t operand;        /* Operand value */
    char label[64];         /* Optional label */
    char comment[128];      /* Optional comment for debugging */
} instruction_t;

typedef struct {
    instruction_t *instructions;    /* Array of instructions */
    int count;                      /* Number of instructions */
    int capacity;                   /* Capacity of array */
    int label_counter;              /* For generating unique labels */
} code_buffer_t;


typedef struct {
    code_buffer_t *code;            /* Generated code */
    symbol_table_t *symtab;         /* Symbol table */
    int loop_break_label;           /* Label for break statement */
    int loop_continue_label;        /* Label for continue statement */
    bool had_error;                 /* Error flag */
    char error_msg[512];            /* Error message */
} codegen_t;

/* Code generator functions */
codegen_t *codegen_create(symbol_table_t *symtab);
void codegen_destroy(codegen_t *gen);
bool codegen_generate(codegen_t *gen, ast_node_t *ast);

/* Instruction emission */
void codegen_emit(codegen_t *gen, opcode_t opcode, int32_t operand);
void codegen_emit_label(codegen_t *gen, const char *label);
int codegen_new_label(codegen_t *gen);
void codegen_patch_jump(codegen_t *gen, int instruction_index, int target);

/* AST code generation */
void codegen_program(codegen_t *gen, ast_node_t *node);
void codegen_function(codegen_t *gen, ast_node_t *node);
void codegen_statement(codegen_t *gen, ast_node_t *node);
void codegen_expression(codegen_t *gen, ast_node_t *node);

/* Output functions */
void codegen_print(codegen_t *gen);
void codegen_write_file(codegen_t *gen, const char *filename);
const char *opcode_to_string(opcode_t opcode);

/* Error handling */
void codegen_error(codegen_t *gen, const char *message);
bool codegen_had_error(codegen_t *gen);

#endif
