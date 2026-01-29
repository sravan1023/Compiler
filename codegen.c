/*
 * Xinu OS Compiler - Code Generator Implementation
 */

#include "codegen.h"
#include "../include/memory.h"
#include <stdio.h>
#include <string.h>

#define INITIAL_CODE_CAPACITY 1024

/* Create code buffer */
static code_buffer_t *code_buffer_create(void) {
    code_buffer_t *buf = (code_buffer_t *)getmem(sizeof(code_buffer_t));
    if (buf == NULL) return NULL;
    
    buf->instructions = (instruction_t *)getmem(sizeof(instruction_t) * INITIAL_CODE_CAPACITY);
    if (buf->instructions == NULL) {
        freemem(buf, sizeof(code_buffer_t));
        return NULL;
    }
    
    buf->count = 0;
    buf->capacity = INITIAL_CODE_CAPACITY;
    buf->label_counter = 0;
    
    return buf;
}

/* Destroy code buffer */
static void code_buffer_destroy(code_buffer_t *buf) {
    if (buf == NULL) return;
    
    if (buf->instructions != NULL) {
        freemem(buf->instructions, sizeof(instruction_t) * buf->capacity);
    }
    
    freemem(buf, sizeof(code_buffer_t));
}

/* Create code generator */
codegen_t *codegen_create(symbol_table_t *symtab) {
    codegen_t *gen = (codegen_t *)getmem(sizeof(codegen_t));
    if (gen == NULL) return NULL;
    
    gen->code = code_buffer_create();
    if (gen->code == NULL) {
        freemem(gen, sizeof(codegen_t));
        return NULL;
    }
    
    gen->symtab = symtab;
    gen->loop_break_label = -1;
    gen->loop_continue_label = -1;
    gen->had_error = false;
    gen->error_msg[0] = '\0';
    
    return gen;
}

/* Destroy code generator */
void codegen_destroy(codegen_t *gen) {
    if (gen == NULL) return;
    
    code_buffer_destroy(gen->code);
    freemem(gen, sizeof(codegen_t));
}

/* Emit instruction */
void codegen_emit(codegen_t *gen, opcode_t opcode, int32_t operand) {
    if (gen == NULL || gen->code == NULL) return;
    
    /* Grow buffer if needed */
    if (gen->code->count >= gen->code->capacity) {
        int new_capacity = gen->code->capacity * 2;
        instruction_t *new_instructions = (instruction_t *)getmem(sizeof(instruction_t) * new_capacity);
        
        memcpy(new_instructions, gen->code->instructions, 
               sizeof(instruction_t) * gen->code->count);
        
        freemem(gen->code->instructions, sizeof(instruction_t) * gen->code->capacity);
        gen->code->instructions = new_instructions;
        gen->code->capacity = new_capacity;
    }
    
    /* Add instruction */
    instruction_t *inst = &gen->code->instructions[gen->code->count++];
    inst->opcode = opcode;
    inst->operand = operand;
    inst->label[0] = '\0';
    inst->comment[0] = '\0';
}

/* Emit label */
void codegen_emit_label(codegen_t *gen, const char *label) {
    codegen_emit(gen, OP_NOP, 0);
    if (gen->code->count > 0) {
        strncpy(gen->code->instructions[gen->code->count - 1].label, 
                label, sizeof(gen->code->instructions[0].label) - 1);
    }
}

/* Generate new label */
int codegen_new_label(codegen_t *gen) {
    return gen->code->label_counter++;
}

/* Patch jump instruction */
void codegen_patch_jump(codegen_t *gen, int instruction_index, int target) {
    if (instruction_index >= 0 && instruction_index < gen->code->count) {
        gen->code->instructions[instruction_index].operand = target;
    }
}

/* Generate code for expression */
void codegen_expression(codegen_t *gen, ast_node_t *node) {
    if (node == NULL) return;
    
    switch (node->type) {
        case NODE_NUMBER:
            codegen_emit(gen, OP_PUSH, (int32_t)node->value.int_val);
            break;
            
        case NODE_IDENTIFIER: {
            symbol_t *sym = symtab_lookup(gen->symtab, node->name);
            if (sym == NULL) {
                codegen_error(gen, "Undefined variable");
                return;
            }
            
            if (sym->scope_level == 0) {
                codegen_emit(gen, OP_LOADG, sym->offset);
            } else {
                codegen_emit(gen, OP_LOADL, sym->offset);
            }
            break;
        }
        
        case NODE_BINARY_OP:
            codegen_expression(gen, node->left);
            codegen_expression(gen, node->right);
            
            if (strcmp(node->op, "+") == 0) {
                codegen_emit(gen, OP_ADD, 0);
            } else if (strcmp(node->op, "-") == 0) {
                codegen_emit(gen, OP_SUB, 0);
            } else if (strcmp(node->op, "*") == 0) {
                codegen_emit(gen, OP_MUL, 0);
            } else if (strcmp(node->op, "/") == 0) {
                codegen_emit(gen, OP_DIV, 0);
            } else if (strcmp(node->op, "%") == 0) {
                codegen_emit(gen, OP_MOD, 0);
            } else if (strcmp(node->op, "&") == 0) {
                codegen_emit(gen, OP_AND, 0);
            } else if (strcmp(node->op, "|") == 0) {
                codegen_emit(gen, OP_OR, 0);
            } else if (strcmp(node->op, "^") == 0) {
                codegen_emit(gen, OP_XOR, 0);
            } else if (strcmp(node->op, "<<") == 0) {
                codegen_emit(gen, OP_SHL, 0);
            } else if (strcmp(node->op, ">>") == 0) {
                codegen_emit(gen, OP_SHR, 0);
            } else if (strcmp(node->op, "==") == 0) {
                codegen_emit(gen, OP_EQ, 0);
            } else if (strcmp(node->op, "!=") == 0) {
                codegen_emit(gen, OP_NE, 0);
            } else if (strcmp(node->op, "<") == 0) {
                codegen_emit(gen, OP_LT, 0);
            } else if (strcmp(node->op, "<=") == 0) {
                codegen_emit(gen, OP_LE, 0);
            } else if (strcmp(node->op, ">") == 0) {
                codegen_emit(gen, OP_GT, 0);
            } else if (strcmp(node->op, ">=") == 0) {
                codegen_emit(gen, OP_GE, 0);
            } else if (strcmp(node->op, "&&") == 0) {
                codegen_emit(gen, OP_LAND, 0);
            } else if (strcmp(node->op, "||") == 0) {
                codegen_emit(gen, OP_LOR, 0);
            }
            break;
            
        case NODE_UNARY_OP:
            codegen_expression(gen, node->left);
            
            if (strcmp(node->op, "-") == 0) {
                codegen_emit(gen, OP_NEG, 0);
            } else if (strcmp(node->op, "!") == 0) {
                codegen_emit(gen, OP_LNOT, 0);
            } else if (strcmp(node->op, "~") == 0) {
                codegen_emit(gen, OP_NOT, 0);
            }
            break;
            
        case NODE_ASSIGN: {
            symbol_t *sym = symtab_lookup(gen->symtab, node->left->name);
            if (sym == NULL) {
                codegen_error(gen, "Undefined variable");
                return;
            }
            
            codegen_expression(gen, node->right);
            codegen_emit(gen, OP_DUP, 0);  /* Duplicate value for return */
            
            if (sym->scope_level == 0) {
                codegen_emit(gen, OP_STOREG, sym->offset);
            } else {
                codegen_emit(gen, OP_STOREL, sym->offset);
            }
            break;
        }
        
        case NODE_CALL: {
            /* Push arguments */
            for (int i = 0; i < node->child_count; i++) {
                codegen_expression(gen, node->children[i]);
            }
            
            symbol_t *sym = symtab_lookup(gen->symtab, node->left->name);
            if (sym == NULL) {
                codegen_error(gen, "Undefined function");
                return;
            }
            
            codegen_emit(gen, OP_CALL, sym->offset);
            break;
        }
        
        case NODE_GETPID:
            codegen_emit(gen, OP_GETPID, 0);
            break;
            
        default:
            break;
    }
}

/* Generate code for statement */
void codegen_statement(codegen_t *gen, ast_node_t *node) {
    if (node == NULL) return;
    
    switch (node->type) {
        case NODE_EXPR_STMT:
            codegen_expression(gen, node->left);
            codegen_emit(gen, OP_POP, 0);  /* Discard result */
            break;
            
        case NODE_RETURN:
            if (node->left != NULL) {
                codegen_expression(gen, node->left);
            } else {
                codegen_emit(gen, OP_PUSH, 0);
            }
            codegen_emit(gen, OP_RET, 0);
            break;
            
        case NODE_IF: {
            int else_label = codegen_new_label(gen);
            int end_label = codegen_new_label(gen);
            
            /* Condition */
            codegen_expression(gen, node->left);
            int jz_index = gen->code->count;
            codegen_emit(gen, OP_JZ, 0);  /* Will be patched */
            
            /* Then branch */
            codegen_statement(gen, node->right);
            
            if (node->extra != NULL) {
                int jmp_index = gen->code->count;
                codegen_emit(gen, OP_JMP, 0);  /* Will be patched */
                
                /* Else branch */
                codegen_patch_jump(gen, jz_index, gen->code->count);
                codegen_statement(gen, node->extra);
                
                codegen_patch_jump(gen, jmp_index, gen->code->count);
            } else {
                codegen_patch_jump(gen, jz_index, gen->code->count);
            }
            break;
        }
        
        case NODE_WHILE: {
            int loop_start = gen->code->count;
            int old_break = gen->loop_break_label;
            int old_continue = gen->loop_continue_label;
            
            gen->loop_continue_label = loop_start;
            
            /* Condition */
            codegen_expression(gen, node->left);
            int jz_index = gen->code->count;
            codegen_emit(gen, OP_JZ, 0);  /* Will be patched */
            
            /* Body */
            codegen_statement(gen, node->right);
            codegen_emit(gen, OP_JMP, loop_start);
            
            /* End */
            codegen_patch_jump(gen, jz_index, gen->code->count);
            gen->loop_break_label = old_break;
            gen->loop_continue_label = old_continue;
            break;
        }
        
        case NODE_FOR: {
            int old_break = gen->loop_break_label;
            int old_continue = gen->loop_continue_label;
            
            /* Init */
            if (node->left != NULL) {
                codegen_expression(gen, node->left);
                codegen_emit(gen, OP_POP, 0);
            }
            
            int loop_start = gen->code->count;
            
            /* Condition */
            if (node->right != NULL) {
                codegen_expression(gen, node->right);
                int jz_index = gen->code->count;
                codegen_emit(gen, OP_JZ, 0);  /* Will be patched */
                gen->loop_break_label = jz_index;
            }
            
            /* Body */
            if (node->child_count > 0) {
                codegen_statement(gen, node->children[0]);
            }
            
            /* Increment */
            int continue_pos = gen->code->count;
            gen->loop_continue_label = continue_pos;
            if (node->extra != NULL) {
                codegen_expression(gen, node->extra);
                codegen_emit(gen, OP_POP, 0);
            }
            
            codegen_emit(gen, OP_JMP, loop_start);
            
            /* Patch break */
            if (node->right != NULL && gen->loop_break_label >= 0) {
                codegen_patch_jump(gen, gen->loop_break_label, gen->code->count);
            }
            
            gen->loop_break_label = old_break;
            gen->loop_continue_label = old_continue;
            break;
        }
        
        case NODE_BREAK:
            if (gen->loop_break_label >= 0) {
                codegen_emit(gen, OP_JMP, gen->loop_break_label);
            }
            break;
            
        case NODE_CONTINUE:
            if (gen->loop_continue_label >= 0) {
                codegen_emit(gen, OP_JMP, gen->loop_continue_label);
            }
            break;
            
        case NODE_BLOCK:
            for (int i = 0; i < node->child_count; i++) {
                codegen_statement(gen, node->children[i]);
            }
            break;
            
        /* Xinu-specific operations */
        case NODE_CREATE:
            for (int i = 0; i < node->child_count; i++) {
                codegen_expression(gen, node->children[i]);
            }
            codegen_emit(gen, OP_CREATE, node->child_count);
            break;
            
        case NODE_RESUME:
            codegen_expression(gen, node->left);
            codegen_emit(gen, OP_RESUME, 0);
            break;
            
        case NODE_SUSPEND:
            codegen_expression(gen, node->left);
            codegen_emit(gen, OP_SUSPEND, 0);
            break;
            
        case NODE_KILL:
            codegen_expression(gen, node->left);
            codegen_emit(gen, OP_KILL, 0);
            break;
            
        case NODE_SLEEP:
            codegen_expression(gen, node->left);
            codegen_emit(gen, OP_SLEEP, 0);
            break;
            
        case NODE_YIELD:
            codegen_emit(gen, OP_YIELD, 0);
            break;
            
        case NODE_WAIT:
            codegen_expression(gen, node->left);
            codegen_emit(gen, OP_WAIT, 0);
            break;
            
        case NODE_SIGNAL:
            codegen_expression(gen, node->left);
            codegen_emit(gen, OP_SIGNAL, 0);
            break;
            
        default:
            break;
    }
}

/* Generate code for function */
void codegen_function(codegen_t *gen, ast_node_t *node) {
    if (node == NULL) return;
    
    /* Emit function label */
    char label[64];
    snprintf(label, sizeof(label), "func_%s", node->name);
    codegen_emit_label(gen, label);
    
    /* Function body */
    if (node->left != NULL) {
        codegen_statement(gen, node->left);
    }
    
    /* Implicit return */
    codegen_emit(gen, OP_PUSH, 0);
    codegen_emit(gen, OP_RET, 0);
}

/* Generate code for program */
void codegen_program(codegen_t *gen, ast_node_t *node) {
    if (node == NULL || node->type != NODE_PROGRAM) return;
    
    for (int i = 0; i < node->child_count; i++) {
        ast_node_t *child = node->children[i];
        
        if (child->type == NODE_FUNCTION || child->type == NODE_PROCESS) {
            codegen_function(gen, child);
        }
    }
    
    /* Halt at end */
    codegen_emit(gen, OP_HALT, 0);
}

/* Main generation entry point */
bool codegen_generate(codegen_t *gen, ast_node_t *ast) {
    if (gen == NULL || ast == NULL) return false;
    
    codegen_program(gen, ast);
    
    return !gen->had_error;
}

/* Convert opcode to string */
const char *opcode_to_string(opcode_t opcode) {
    switch (opcode) {
        case OP_PUSH:    return "PUSH";
        case OP_POP:     return "POP";
        case OP_DUP:     return "DUP";
        case OP_ADD:     return "ADD";
        case OP_SUB:     return "SUB";
        case OP_MUL:     return "MUL";
        case OP_DIV:     return "DIV";
        case OP_MOD:     return "MOD";
        case OP_NEG:     return "NEG";
        case OP_AND:     return "AND";
        case OP_OR:      return "OR";
        case OP_XOR:     return "XOR";
        case OP_NOT:     return "NOT";
        case OP_SHL:     return "SHL";
        case OP_SHR:     return "SHR";
        case OP_LAND:    return "LAND";
        case OP_LOR:     return "LOR";
        case OP_LNOT:    return "LNOT";
        case OP_EQ:      return "EQ";
        case OP_NE:      return "NE";
        case OP_LT:      return "LT";
        case OP_LE:      return "LE";
        case OP_GT:      return "GT";
        case OP_GE:      return "GE";
        case OP_LOAD:    return "LOAD";
        case OP_STORE:   return "STORE";
        case OP_LOADL:   return "LOADL";
        case OP_STOREL:  return "STOREL";
        case OP_LOADG:   return "LOADG";
        case OP_STOREG:  return "STOREG";
        case OP_JMP:     return "JMP";
        case OP_JZ:      return "JZ";
        case OP_JNZ:     return "JNZ";
        case OP_CALL:    return "CALL";
        case OP_RET:     return "RET";
        case OP_CREATE:  return "CREATE";
        case OP_RESUME:  return "RESUME";
        case OP_SUSPEND: return "SUSPEND";
        case OP_KILL:    return "KILL";
        case OP_SLEEP:   return "SLEEP";
        case OP_YIELD:   return "YIELD";
        case OP_WAIT:    return "WAIT";
        case OP_SIGNAL:  return "SIGNAL";
        case OP_GETPID:  return "GETPID";
        case OP_NOP:     return "NOP";
        case OP_HALT:    return "HALT";
        default:         return "UNKNOWN";
    }
}

/* Print generated code */
void codegen_print(codegen_t *gen) {
    if (gen == NULL || gen->code == NULL) return;
    
    printf("Generated code (%d instructions):\n", gen->code->count);
    
    for (int i = 0; i < gen->code->count; i++) {
        instruction_t *inst = &gen->code->instructions[i];
        
        if (inst->label[0] != '\0') {
            printf("%s:\n", inst->label);
        }
        
        printf("  %04d: %-10s %d", i, opcode_to_string(inst->opcode), inst->operand);
        
        if (inst->comment[0] != '\0') {
            printf("  ; %s", inst->comment);
        }
        
        printf("\n");
    }
}

/* Error handling */
void codegen_error(codegen_t *gen, const char *message) {
    gen->had_error = true;
    strncpy(gen->error_msg, message, sizeof(gen->error_msg) - 1);
    gen->error_msg[sizeof(gen->error_msg) - 1] = '\0';
}

bool codegen_had_error(codegen_t *gen) {
    return gen->had_error;
}
