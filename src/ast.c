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
    generated_ast = realloc(generated_ast, sizeof(ASTNode*) * (ast_length + 1));
    if (!generated_ast) {
        perror("realloc failed");
        exit(EXIT_FAILURE);
    }
    generated_ast[ast_length] = node;
    ast_length++;
}

static inline void print_indent(int depth) {
    if (depth == 0) return;
    for (int i = 0; i < depth; i++) printf("\t");
}

static inline void print_node(ASTNode* node, int depth) {
    if (!node) { print_indent(depth); printf("NODE IS NULL\n"); return; }

    if (node->type == STOP) return;

    print_indent(depth);
    if (node->type >= 0 && 
        node->type < sizeof(ASTNodeTypeNames)/sizeof(ASTNodeTypeNames[0]) )
        printf("Type: %s\n", ASTNodeTypeNames[node->type]);
    else
        fatal_error("Type: UNKNOWN (%d)\n", node->type);

    switch (node->type) {
        case GLOBAL_DECLARATION:
            print_indent(depth);
            printf("Title: %s\n", node->list.title);
            print_node(node->list.next, depth + 1);
            break;
        case AUTO:
        case EXTRN: print_node(node->list.next, depth + 1); break;
        case ASSIGNMENT:
            print_indent(depth);
            printf("Title: %s\n", node->list.title);
            print_node(node->list.next, depth + 1);
            break;
        case WHILE_LOOP:
            print_node(node->list.inner, depth + 1);
            print_node(node->list.next, depth + 1);
            break;
        case IF:
            print_node(node->if_t.cond, depth + 1);
            print_node(node->if_t.statements, depth + 1);
            print_node(node->if_t.else_t, depth + 1);
            break;
        case LABEL:
            print_indent(depth);
            printf("Title: %s\n", node->string);
            break;
        case RETURN:
            print_node(node->list.next, depth + 1);
            break;
        case FUNCTION:
            print_indent(depth);
            printf("Title: %s\n", node->function.title);
            print_node(node->function.args, depth + 1);
            print_node(node->function.statements, depth + 1);
            break;

        case ADD:
        case SUBTRACT:
        case MULTIPLY:
        case DIVIDE:
        case GTEQ:
        case LTEQ:
        case GREATER:
        case LESS:
        case EQUALS:
        case NEQUALS:
            print_node(node->factors.left, depth + 1);
            print_node(node->factors.right, depth + 1);
            break;
        case FUNCTION_CALL:
            print_indent(depth);
            printf("Title: %s\n", node->list.title);
            print_node(node->list.next, depth + 1);
            break;
        
        case NOT:
            print_node(node->inner, depth + 1);
            break;

        case INC:
        case DEC:
            print_indent(depth);
            printf("Title: %s\n", node->string);
            break;

        case NUMBER:
            print_indent(depth);
            printf("Value: %d\n", node->integer);
            break;
        case VARIABLE:
            print_indent(depth);
            printf("Title: %s\n", node->string);
            print_node(node->list.next, depth);
            break;
        case ARRAY:
            print_node(node->list.inner, depth + 1);
            print_node(node->list.next, depth);
            break;
        case ARRAY_REF:
            print_indent(depth);
            printf("Title: %s\n", node->list.title);
            print_node(node->list.next, depth + 1);
            break;
        default:
            print_indent(depth);
            printf("Unreachable code.\n");
            break;
    }

    if (node->successor) print_node(node->successor, depth);
}

void print_ast() {
    for (int i = 0; i < ast_length; i++) print_node(generated_ast[i], 0);
    printf("\n");
}