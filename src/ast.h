/*
   BLang
   Copyright (c) 2025 William Gibbs

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
      claim that you wrote the original software. If you use this software
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original software.
   3. This notice may not be removed or altered from any source distribution.
*/

#ifndef AST_H
#define AST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VariableType {
    VAR_AUTO, VAR_EXTRN
} VariableType;

typedef struct ASTNode {
    enum { 
        GLOBAL_DECLARATION, AUTO, EXTRN,
        ASSIGNMENT,
        WHILE_LOOP,
        SWITCH_STMT, SWITCH_CASE,
        IF,
        LABEL, GOTO,
        FUNCTION, FUNCTION_CALL, RETURN, 
        ADD, SUB, MUL, DIV,
        GE, LE, GT, LT, EQ, NE, NOT,
        NEGATIVE,
        INC, DEC,
        NUMBER, VARIABLE, STRING, ARRAY, ARRAY_REF,
        STOP
    } type;
    int val;
    char* str;
    int childC;
    struct ASTNode** childV;
    VariableType variableType;
    struct ASTNode* successor;
} ASTNode;

static const char* ASTNodeTypeNames[] = {
    "GLOBAL_DECLARATION", "AUTO", "EXTRN",
    "ASSIGNMENT",
    "WHILE_LOOP",
    "SWITCH_STMT", "SWITCH_CASE",
    "IF",
    "LABEL", "GOTO",
    "FUNCTION", "FUNCTION_CALL", "RETURN", 
    "ADD", "SUB", "MUL", "DIV",
    "GE", "LE", "GT", "LT", "EQ", "NE", "NOT",
    "NEGATIVE",
    "INC", "DEC",
    "NUMBER", "VARIABLE", "STRING", "ARRAY", "ARRAY_REF",
    "STOP"
};

extern ASTNode** generated_ast;
extern int ast_length;

extern void append_statement(ASTNode* node);

extern void print_ast();

#ifdef __cplusplus
}
#endif

#endif // AST_H