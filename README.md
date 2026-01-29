# Compiler Module

## Overview

The Compiler module provides a complete compilation infrastructure for the Xinu operating system, enabling the compilation of source code into executable bytecode. This module implements a full compiler pipeline with lexical analysis, syntax parsing, semantic analysis, and code generation capabilities.

## Architecture

The compiler is structured as a multi-phase pipeline consisting of four primary components:

### Components

#### 1. Lexical Analyzer (Lexer)
**Files:** `lexer.c`, `lexer.h`

The lexical analyzer performs tokenization of source code, converting raw text into a stream of tokens. It supports:

- **Literals**: Integers (decimal, hexadecimal, binary), floating-point numbers, strings, and characters
- **Operators**: Arithmetic, bitwise, logical, and comparison operators
- **Keywords**: Language keywords including control flow, type declarations, and Xinu-specific constructs
- **Identifiers**: Variable and function names
- **Punctuation**: Delimiters, separators, and structural symbols

The lexer provides comprehensive error reporting with line and column tracking for precise diagnostics.

#### 2. Parser
**Files:** `parser.c`, `parser.h`

The parser implements a recursive descent parsing algorithm that constructs an Abstract Syntax Tree (AST) from the token stream. Supported language constructs include:

- **Program Structure**: Function definitions, Xinu process definitions, system calls, interrupt handlers
- **Declarations**: Variables, arrays, structures, unions, enumerations, type definitions
- **Statements**: Control flow (if, while, for, switch), jumps (break, continue, return, goto), labels
- **Expressions**: Binary and unary operations, function calls, array indexing, member access, assignments
- **Types**: Primitive types, pointers, arrays, structures, unions, function types

The parser performs syntax validation and generates a hierarchical representation of the program structure.

#### 3. Symbol Table
**Files:** `symbol_table.c`, `symbol_table.h`

The symbol table manages identifier information throughout the compilation process. It provides:

- Scoped symbol management with nested scope support
- Type information tracking
- Variable and function declaration storage
- Symbol resolution and lookup functionality
- Semantic validation support

#### 4. Code Generator
**Files:** `codegen.c`, `codegen.h`

The code generator traverses the AST and emits bytecode instructions. It implements:

- **Stack-based virtual machine instructions**
- **Instruction Set**:
  - Stack operations (push, pop, duplicate, swap)
  - Arithmetic operations (add, subtract, multiply, divide, modulo)
  - Bitwise operations (AND, OR, XOR, NOT, shifts)
  - Logical operations (AND, OR, NOT)
  - Comparison operations (equal, not equal, less than, greater than, etc.)
  - Memory operations (load, store, local variable access)
  - Control flow (jumps, conditional branches, function calls)
  - System calls and process operations

The code generator produces optimized bytecode suitable for execution on the Xinu virtual machine.

## Compiler Pipeline

The compilation process follows a standard multi-phase pipeline:

1. **Lexical Analysis**: Source code -> Token stream
2. **Syntax Analysis**: Token stream -> Abstract Syntax Tree (AST)
3. **Semantic Analysis**: AST validation and symbol table construction
4. **Code Generation**: AST -> Bytecode instructions

## Compiler Interface

**Files:** `compiler.c`, `compiler.h`

The main compiler interface provides a unified API for the entire compilation process:

### Compiler Options

The compiler supports various configuration options:
- Token dumping for debugging
- AST visualization
- Symbol table printing
- Generated code listing
- Optimization levels
- Output file specification

## Error Handling

The compiler provides comprehensive error reporting with:
- Precise error location (line and column numbers)
- Descriptive error messages
- Warning levels for code quality issues
- Error recovery mechanisms for continued compilation
- Separate error and warning counters

## Integration

The compiler module integrates with other Xinu components:
- **Interpreter Module**: Executes compiled bytecode
- **Process Management**: Supports process and system call definitions
- **Memory Management**: Generates code for memory operations

## Usage

The compiler is designed to compile code for the Xinu operating system environment, supporting:
- Process definitions specific to Xinu
- System call implementations
- Interrupt handler code
- General-purpose C-like code with Xinu extensions


## Technical Specifications

- **Maximum Token Length**: 256 characters
- **Maximum String Length**: 1024 characters
- **Maximum Function Parameters**: 32
- **Maximum Local Variables**: 256
- **Maximum Struct Members**: 64
- **Maximum Array Dimensions**: 8