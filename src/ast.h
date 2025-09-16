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
        _GLOBAL_DECLARATION,
        _AUTO,
        _EXTRN,
        _ASSIGNMENT,
        _WHILE_LOOP,
        _IF,
        _LABEL,
        _RETURN,
        _GOTO,
        _FUNCTION,

        
        _ADD,
        _SUBTRACT,
        _MULTIPLY,
        _DIVIDE,
        _GTEQ,
        _LTEQ,
        _GREATER,
        _LESS,
        _EQUALS,
        _NEQUALS,
        _FUNCTION_CALL,

        _NOT,
        _NEGATIVE,

        _INC,
        _DEC,

        _NUMBER,
        _VARIABLE,
        _ARRAY,
        _ARRAY_REF,

        STOP
    } type;
    union {
        int integer;

        char* string;

        struct ASTNode* inner;

        struct {
            struct ASTNode* left;
            struct ASTNode* right;
        } factors;

        struct {
            union {
                char* title;
                struct ASTNode* inner;
            };
            struct ASTNode* next;
            VariableType variableType;
        } list;

        struct {
            char* title;
            struct ASTNode* args;
            struct ASTNode* statements;
        } function;

        struct {
            struct ASTNode* cond;
            struct ASTNode* statements;
            struct ASTNode* else_t;
        } if_t;
    };
    struct ASTNode* successor;
} ASTNode;

static const char* ASTNodeTypeNames[] = {
    "_GLOBAL_DECLARATION",
    "_AUTO",
    "_EXTRN",
    "_ASSIGNMENT",
    "_WHILE_LOOP",
    "_IF",
    "_LABEL",
    "_RETURN",
    "_GOTO",
    "_FUNCTION",

    "_ADD",
    "_SUBTRACT",
    "_MULTIPLY",
    "_DIVIDE",
    "_GTEQ",
    "_LTEQ",
    "_GREATER",
    "_LESS",
    "_EQUALS",
    "_NEQUALS",
    "_FUNCTION_CALL",

    "_NOT",

    "_INC",
    "_DEC",

    "_NUMBER",
    "_VARIABLE",
    "_ARRAY",
    "_ARRAY_REF",

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