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

%define parse.error verbose
%define parse.lac full

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "error.h"

extern void yyerror(const char *s);
extern int yylex(void);
extern int yyparse(void);
extern int yy_scan_string(const char *str);

// Create an allocation for a new node.
static ASTNode* new_node(int type) {
   ASTNode* output = malloc(sizeof(ASTNode));
   if (!output) fatal_error("failed to allocate space for an AST node.");
   output->type = type;
   output->childC = 0;
   output->childV = NULL;
   return output;
}

static void add_child(ASTNode* parent, ASTNode* child) {
   ASTNode** tmp = realloc(
      parent->childV,
      (parent->childC + 1) * sizeof(*parent->childV)
   );

   if (tmp == NULL) return;

   parent->childV = tmp;
   parent->childV[parent->childC] = child;
   parent->childC++;
}

%}

%union {
   int val;
   char* str;
   struct ASTNode* node;
}

%token <str> F_IDENTIFIER F_STRING
%token F_AUTO F_EXTRN
%token <val> F_NUMBER F_CHAR F_ARRAY
%token F_PLUSEQ F_MINUSEQ F_TIMESEQ F_DIVEQ F_INC F_DEC
%token F_EQ F_NE F_GE F_LE F_AND F_OR
%token F_LSHIFT F_RSHIFT
%token F_WHILE F_IF F_ELSE F_SWITCH F_CASE F_DEFAULT
%token F_GOTO F_RETURN

%type <node> function 
%type <node> statement_list statement 
%type <node> expression parameters
%type <node> block switch_body;

%left '+' '-'
%left '*' '/'

%nonassoc F_GE F_LE '>' '<' F_EQ F_NE

%%

program:
   /* empty */ 
   |  program F_IDENTIFIER expression ';' {
      ASTNode* node = new_node(GLOBAL_DECLARATION);
      node->str = $2;
      add_child(node, $3);
      append_statement(node);
   }
   |  program function { append_statement($2); }
   ;

function:
   F_IDENTIFIER '(' parameters ')' '{' statement_list '}' { 
      ASTNode* node = new_node(FUNCTION);
      node->str = $1;
      add_child(node, $3);
      add_child(node, $6);
      $$ = node;
   }
   ;

statement_list:
   /* empty */ {
      ASTNode* node = new_node(STOP);
      $$ = node;
   }
   |  statement statement_list {
      ASTNode* node = $1;
      node->successor = $2;
      $$ = node;
   }
   ;

statement:

   F_AUTO parameters ';' { 
      ASTNode* node = new_node(AUTO);
      add_child(node, $2);

      // Set all variables defined to be of type "AUTO"
      ASTNode* current = node; // TODO: This looks sketchy.
      while (current->type != STOP) {
         current->variableType = VAR_AUTO;
         current = current->childV[0];
      }

      $$ = node;
   }

   |  F_EXTRN parameters ';' { 
      ASTNode* node = new_node(EXTRN);
      add_child(node, $2);

      // Set all variables defined to be of type "EXTRN"
      ASTNode* current = node;
      while (current->type != STOP) {
         current->variableType = VAR_EXTRN;
         current = current->childV[0];
      }

      $$ = node;
   }

   |  F_IDENTIFIER '=' expression ';' { 
      ASTNode* node = new_node(ASSIGNMENT);
      node->str = $1;
      add_child(node, $3);
      $$ = node;
   }

   |  F_WHILE '(' expression ')' block { 
      ASTNode* node = new_node(WHILE_LOOP);
      add_child(node, $3);
      add_child(node, $5);
      $$ = node;
   }

   |  F_SWITCH '(' expression ')' switch_body {
      ASTNode* node = new_node(SWITCH_STMT);
      node->childC++;
      add_child(node, $3);
      add_child(node, $5);
      $$ = node;
   }
   
   |  F_IF '(' expression ')' block { 
      ASTNode* node = new_node(IF);
      node->childV[0] = $3;
      node->childV[1] = $5;
      node->childV[2] = new_node(STOP);
      $$ = node;
   }

   |  F_IF '(' expression ')' block F_ELSE block { 
      ASTNode* node = new_node(IF);
      node->childV[0] = $3;
      node->childV[1] = $5;
      node->childV[2] = $7;
      $$ = node;
   }

   | F_IDENTIFIER ':' { 
     ASTNode* node = new_node(LABEL);
      node->str = $1;
      $$ = node;
   }

   | F_IDENTIFIER '(' parameters ')' ';' { 
      ASTNode* node = new_node(FUNCTION_CALL);
      node->str = $1;
      add_child(node, $3);
      $$ = node;
   }

   | F_IDENTIFIER F_INC ';' {
      ASTNode* node = new_node(INC);
      node->str = $1;
      $$ = node;
   }
   | F_INC F_IDENTIFIER ';' {
      ASTNode* node = new_node(INC);
      node->str = $2;
      $$ = node;
   }
      
   | F_IDENTIFIER F_DEC ';' {
      ASTNode* node = new_node(DEC);
      node->str = $1;
      $$ = node;
   }

   | F_DEC F_IDENTIFIER ';' {
      ASTNode* node = new_node(DEC);
      node->str = $2;
      $$ = node;
   }

   | F_RETURN '(' expression ')' ';' { 
      ASTNode* node = new_node(RETURN);
      add_child(node, $3);
      $$ = node;
   }

   | F_GOTO F_IDENTIFIER ';' {
      ASTNode* node = new_node(GOTO);
      node->str = $2;
      $$ = node;
   }
   ;

expression:

   '(' expression ')' { $$ = $2; }

   | '!' expression {
      ASTNode* node = new_node(NOT);
      node->childV[0] = $2;
      $$ = node;
   }

   | '-' expression {
      ASTNode* negative = new_node(NUMBER);
      negative->val = -1; // Multiply by -1.

      ASTNode* node = new_node(MUL);
      node->childV[0] = $2;
      node->childV[1] = negative;
      $$ = node;
   }

   |  expression '+' expression {  
      ASTNode* node = new_node(ADD);
      add_child(node, $1);
      add_child(node, $3);
      $$ = node;
   }

   |  expression '-' expression {  
      ASTNode* node = new_node(SUB);
      add_child(node, $1);
      add_child(node, $3);
      $$ = node;
   }

   |  expression '*' expression {  
      ASTNode* node = new_node(MUL);
      add_child(node, $1);
      add_child(node, $3);
      $$ = node;
   }

   |  expression '/' expression {  
      ASTNode* node = new_node(DIV);
      add_child(node, $1);
      add_child(node, $3);
      $$ = node;
   }

   |  expression F_GE expression {  
      ASTNode* node = new_node(GE);
      add_child(node, $1);
      add_child(node, $3);
      $$ = node;
   }

   |  expression F_LE expression {  
      ASTNode* node = new_node(LE);
      add_child(node, $1);
      add_child(node, $3);
      $$ = node;
   }

   |  expression '>' expression {  
      ASTNode* node = new_node(GT);
      add_child(node, $1);
      add_child(node, $3);
      $$ = node;
   }

   |  expression '<' expression {  
      ASTNode* node = new_node(LT);
      add_child(node, $1);
      add_child(node, $3);
      $$ = node;
   }

   |  expression F_EQ expression {  
      ASTNode* node = new_node(EQ);
      add_child(node, $1);
      add_child(node, $3);
      $$ = node;
   }

   |  expression F_NE expression {  
      ASTNode* node = new_node(NE);
      add_child(node, $1);
      add_child(node, $3);
      $$ = node;
   }

   |  F_IDENTIFIER '(' parameters ')' {
      ASTNode* node = new_node(FUNCTION_CALL);
      node->str = $1;
      add_child(node, new_node(STOP));
      add_child(node, $3);
      $$ = node;    
   }
      
   |  F_IDENTIFIER F_INC {
      ASTNode* node = new_node(INC);
      node->str = $1;
      $$ = node;
   }
   
   |  F_INC F_IDENTIFIER {
      ASTNode* node = new_node(INC);
      node->str = $2;
      $$ = node;
   }
      
   |  F_IDENTIFIER F_DEC {
      ASTNode* node = new_node(DEC);
      node->str = $1;
      $$ = node;
   }
   
   |  F_DEC F_IDENTIFIER {
      ASTNode* node = new_node(DEC);
      node->str = $2;
      $$ = node;
   }
   
   |  F_IDENTIFIER {
      ASTNode* node = new_node(VARIABLE);
      node->str = $1;
      $$ = node;
   }

   | F_STRING {
      ASTNode* node = new_node(STRING);
      node->str = $1;
      $$ = node;
   }

   | F_NUMBER {
      ASTNode* node = new_node(NUMBER);
      node->val = $1;
      $$ = node;
   }

   | F_CHAR {
      ASTNode* node = new_node(NUMBER);
      node->val = (int)$1;
      $$ = node;
   }
   ;

block:
   '{' statement_list '}' { $$ = $2; }
   | statement { $$ = $1; }
   ;

switch_body:
   F_CASE expression ':' block switch_body {
      ASTNode* node = new_node(SWITCH_CASE);
   }
   | F_DEFAULT expression ':' block {
      ASTNode* end = new_node(STOP);
      ASTNode* node = new_node(SWITCH_CASE);
   }
   ;

parameters:
   /* empty */ {
      ASTNode* node = new_node(STOP);
      $$ = node;
   }
   | expression {
      ASTNode* node = new_node(STOP);
      $1->successor = node;
      $$ = $1;
   }
   | parameters ',' expression {
      $3->successor = $1;
      $$ = $1;
   }

%%

void yyerror(const char *s) { fprintf(stderr, "Parsing Error: %s\n", s); }