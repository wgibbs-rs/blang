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

#include "ast.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>

ASTNode** generated_ast = NULL;
int ast_length = 0;

void append_statement(ASTNode* node) {
    ASTNode** tmp = realloc(
        generated_ast,
        sizeof(*generated_ast) * (ast_length + 1)
    );

    if (!tmp) fatal_error("failed to allocate generated AST");

    generated_ast = tmp;
    generated_ast[ast_length++] = node;
}

static inline void print_indent(int depth) {
    for (int i = 0; i < depth; i++) printf("  ");
}

static inline void print_node(ASTNode* node, int depth) {
    if (!node || node->type == STOP) { return; }

    if (node->type >= 0 && 
        node->type < sizeof(ASTNodeTypeNames)/sizeof(ASTNodeTypeNames[0]) ) {
        print_indent(depth);
        printf("%s\n", ASTNodeTypeNames[node->type]);
    } else fatal_error("invalid type id: %d\n", node->type);

    // int val;
    print_indent(depth + 1);
    printf("Short Value: %d\n", node->val);
    // char* str;
    print_indent(depth + 1);
    printf("String Value: \"%s\"\n", node->str);
    // VariableType variableType;
    print_indent(depth + 1);
    printf("VariableType: %d\n", node->variableType);
    // int childC;
    // struct ASTNode** childV;
    print_indent(depth + 1);
    printf("Children: %d\n", node->childC);
    for (int i = 0; i < node->childC; i++)
        print_node(node->childV[i], depth + 2);
    
    if (node->successor) print_node(node->successor, depth);
}

void print_ast() {
    for (int i = 0; i < ast_length; i++) print_node(generated_ast[i], 0);
    printf("\n");
}