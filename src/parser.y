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

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "error.h"
#include "opt.h"

extern void yyerror(const char *s);
extern int yylex(void);
extern int yyparse(void);
extern int yy_scan_string(const char *str);

// Create an allocation for a new node.
static ASTNode* new_node(int type) {
   ASTNode* output = malloc(sizeof(ASTNode));
   if (GCC_UNLIKELY(!output)) 
      fatal_error("failed to allocate space for an AST node.");
   output->type = type;
   return output;
}

%}


%union {
   int integer;
   char* str;
   char character;
   struct ASTNode* node;
}

%token <str> F_IDENTIFIER F_STRING
%token F_AUTO F_EXTRN
%token <integer> F_NUMBER F_CHAR F_ARRAY
%token F_PLUSEQ F_MINUSEQ F_TIMESEQ F_DIVEQ F_INC F_DEC
%token F_EQ F_NEQ F_GTEQ F_LTEQ F_AND F_OR
%token F_LSHIFT F_RSHIFT
%token F_WHILE F_IF F_ELSE F_SWITCH F_CASE F_DEFAULT
%token F_GOTO F_RETURN

%type <node> function 
%type <node> statement_list statement 
%type <node> expression declaration parameters
%type <node> array_decl else block switch_body;

%left '+' '-'
%left '*' '/'

%nonassoc F_GTEQ F_LTEQ

%%

program:
   /* empty */ 
   |  program F_IDENTIFIER expression ';' {
      ASTNode* node = new_node(GLOBAL_DECLARATION);
      node->list.title = $2;
      node->list.next = $3;
      append_statement(node);
   }
   |  program function { append_statement($2); }
   ;

function:
   F_IDENTIFIER '(' declaration ')' '{' statement_list '}' { 
      ASTNode* node = new_node(FUNCTION);
      node->function.title = $1;
      node->function.args = $3;
      node->function.statements = $6;
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
   F_AUTO declaration ';' { 
      ASTNode* node = new_node(AUTO);
      node->list.next = $2;

      // Set all variables defined to be of type "AUTO"
      ASTNode* current = node; // TODO: This looks sketchy.
      while (current->type != STOP) {
         current->list.variableType = VAR_AUTO;
         current = current->list.next;
      }

      $$ = node;
   }
   |  F_EXTRN declaration ';' { 
      ASTNode* node = new_node(EXTRN);
      node->list.next = $2;

      // Set all variables defined to be of type "EXTRN"
      ASTNode* current = node;
      while (current->type != STOP) {
         current->list.variableType = VAR_EXTRN;
         current = current->list.next;
      }

      $$ = node;
   }
   |  F_IDENTIFIER '=' expression ';' { 
      ASTNode* node = new_node(ASSIGNMENT);
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;
   }

   |  F_WHILE '(' expression ')' block { 
      ASTNode* node = new_node(WHILE_LOOP);
      node->list.inner = $3;
      node->list.next = $5;
      $$ = node;
   }

   |  F_SWITCH '(' expression ')' switch_body {
      ASTNode* node = new_node(SWITCH_STMT);
      node->list.inner = $3;
      node->list.next = $5;
      $$ = node;
   }
   
   |  F_IF '(' expression ')' block else { 
      ASTNode* node = new_node(IF);
      node->if_t.cond = $3;
      node->if_t.statements = $5;
      node->if_t.else_t = $6;
      $$ = node;
   }

   |  F_IDENTIFIER ':' { 
     ASTNode* node = new_node(LABEL);
      node->string = $1;
      $$ = node;
   }

   |  F_IDENTIFIER '(' parameters ')' ';' { 
      ASTNode* node = new_node(FUNCTION_CALL);
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;
   }

   |  F_IDENTIFIER F_INC ';' {
      ASTNode* node = new_node(INC);
      node->string = $1;
      $$ = node;
   }
   |  F_INC F_IDENTIFIER ';' {
      ASTNode* node = new_node(INC);
      node->string = $2;
      $$ = node;
   }
      
   |  F_IDENTIFIER F_DEC ';' {
      ASTNode* node = new_node(DEC);
      node->string = $1;
      $$ = node;
   }

   |  F_DEC F_IDENTIFIER ';' {
      ASTNode* node = new_node(DEC);
      node->string = $2;
      $$ = node;
   }

   |  F_RETURN '(' expression ')' ';' { 
      ASTNode* node = new_node(RETURN);
      node->list.next = $3;
      $$ = node;
   }

   |  F_GOTO F_IDENTIFIER ';' {
      ASTNode* node = new_node(GOTO);
      node->string = $2;
      $$ = node;
   }
   ;

expression:

   '(' expression ')' { $$ = $2; }

   | '!' expression {
      ASTNode* node = new_node(NOT);
      node->inner = $2;
      $$ = node;
   }

   | '-' expression {
      ASTNode* negative = new_node(NUMBER);
      negative->integer = -1; // Multiply by -1.

      ASTNode* node = new_node(MULTIPLY);
      node->factors.left = $2;
      node->factors.right = negative;
      $$ = node;
   }

   |  expression '+' expression {  
      ASTNode* node = new_node(ADD);
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '-' expression {  
      ASTNode* node = new_node(SUBTRACT);
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '*' expression {  
      ASTNode* node = new_node(MULTIPLY);
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '/' expression {  
      ASTNode* node = new_node(DIVIDE);
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression F_GTEQ expression {  
      ASTNode* node = new_node(GE);
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression F_LTEQ expression {  
      ASTNode* node = new_node(LE);
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '>' expression {  
      ASTNode* node = new_node(GT);
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '<' expression {  
      ASTNode* node = new_node(LT);
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression F_EQ expression {  
      ASTNode* node = new_node(EQ);
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression F_NEQ expression {  
      ASTNode* node = new_node(NE);
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  F_IDENTIFIER '(' declaration ')' ';' {
      ASTNode* node = new_node(FUNCTION_CALL);
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;    
   }
      
   |  F_IDENTIFIER F_INC {
      ASTNode* node = new_node(INC);
      node->string = $1;
      $$ = node;
   }
   |  F_INC F_IDENTIFIER {
      ASTNode* node = new_node(INC);
      node->string = $2;
      $$ = node;
   }
      
   |  F_IDENTIFIER F_DEC {
      ASTNode* node = new_node(DEC);
      node->string = $1;
      $$ = node;
   }
   |  F_DEC F_IDENTIFIER {
      ASTNode* node = new_node(DEC);
      node->string = $2;
      $$ = node;
   }
   
   |  F_IDENTIFIER {
      ASTNode* node = new_node(VARIABLE);
      node->string = $1;
      $$ = node;
   }

   |  F_IDENTIFIER array_decl { 
      ASTNode* node = new_node(ARRAY_REF);
      node->list.title = $1;
      node->list.next = $2;
      $$ = node;
   }

   | F_STRING {
      ASTNode* node = new_node(STRING);
      node->string = $1;
      $$ = node;
   }

   |  F_NUMBER {
      ASTNode* node = new_node(NUMBER);
      node->integer = $1;
      $$ = node;
   }

   |  F_CHAR {
      ASTNode* node = new_node(NUMBER);
      node->integer = (int)$1;
      $$ = node;
   }
   ;

declaration:
   /* empty */ {
      ASTNode* node = new_node(STOP);
      $$ = node;
   }
   |  F_IDENTIFIER {
      ASTNode* node = new_node(VARIABLE);
      node->list.title = $1;
      node->list.next = new_node(STOP);
      $$ = node;
   }
   |  F_IDENTIFIER ',' declaration {
      ASTNode* node = new_node(VARIABLE);
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;
   }
;


else:
   /* empty */ {
      ASTNode* node = new_node(STOP);
      $$ = node;
   }
   | F_ELSE block { $$ = $2; }
   ;

block:
   '{' statement_list '}' { $$ = $2; }
   | statement { $$ = $1; }
   ;

switch_body:
   '{' switch_body '}' { $$ = $2; }
   | F_CASE expression ':' block switch_body {
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
   | expression ',' parameters {
      $1->successor = $3;
      $$ = $1;
   }


array_decl:
   /* empty */ {
      ASTNode* node = new_node(STOP);
      $$ = node;
   }
  | '[' expression ']' {
      ASTNode* node = new_node(ARRAY);
      node->list.inner = $2;
      node->list.next = new_node(STOP);
      $$ = node;
    }
  | '[' expression ']' array_decl {
      ASTNode* node = new_node(ARRAY);
      node->list.inner = $2;
      node->list.next = $4;
      $$ = node;
    }
;

%%

void yyerror(const char *s) { fprintf(stderr, "Error: %s\n", s); }