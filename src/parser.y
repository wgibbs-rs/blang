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
#include "ast.h"
#include "error.h"
#include "opt.h"

extern void yyerror(const char *s);
extern int yylex(void);
extern int yyparse(void);
extern int yy_scan_string(const char *str);

static void malloc_err() { fatal_error("failed to allocate space for an AST node."); }
%}


%union {
   int      integer;
   char*    str;
   char     character;

   struct ASTNode* node;
}


%token <str> F_IDENTIFIER
%token F_AUTO F_EXTRN
%token <integer> F_NUMBER F_CHARACTER F_ARRAY
%token <str> F_STRING
%token F_PLUSEQ F_MINUSEQ F_TIMESEQ F_DIVEQ F_INC F_DEC
%token F_EQ F_NEQ F_GTEQ F_LTEQ F_AND F_OR
%token F_LSHIFT F_RSHIFT
%token F_WHILE F_IF F_ELSE
%token F_GOTO
%token F_RETURN


%type <node> function 
%type <node> statement_list statement 
%type <node> expression declaration parameters
%type <node> array_reference else block;

%left '+' '-'
%left '*' '/'

%%

program:
   /* empty */ 
   |  program F_IDENTIFIER expression ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = GLOBAL_DECLARATION;
      node->list.title = $2;
      node->list.next = $3;
      append_statement(node);
   }
   |  program function { append_statement($2); }
   ;

function:
   F_IDENTIFIER '(' declaration ')' '{' statement_list '}' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = FUNCTION;
      node->function.title = $1;
      node->function.args = $3;
      node->function.statements = $6;
      $$ = node;
   }
   ;


statement_list:
   /* empty */ {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = STOP;
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
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = AUTO;
      node->list.next = $2;

      // Set all variables defined to be of type "AUTO"
      ASTNode* current = node;
      while (current->type != STOP) {
         current->list.variableType = VAR_AUTO;
         current = current->list.next;
      }

      $$ = node;
   }
   |  F_EXTRN declaration ';' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = EXTRN;
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
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = ASSIGNMENT;
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;
   }

   |  F_WHILE '(' expression ')' block { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = WHILE_LOOP;
      node->list.inner = $3;
      node->list.next = $5;
      $$ = node;
   }
   
   |  F_IF '(' expression ')' block else { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = IF;
      node->if_t.cond = $3;
      node->if_t.statements = $5;
      node->if_t.else_t = $6;
      $$ = node;
   }

   |  F_IDENTIFIER ':' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = LABEL;
      node->string = $1;
      $$ = node;
   }

   |  F_IDENTIFIER '(' parameters ')' ';' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = FUNCTION_CALL;
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;
   }


   |  F_IDENTIFIER F_INC ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = INC;
      node->string = $1;
      $$ = node;
   }
   |  F_INC F_IDENTIFIER ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = INC;
      node->string = $2;
      $$ = node;
   }
      
   |  F_IDENTIFIER F_DEC ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = DEC;
      node->string = $1;
      $$ = node;
   }
   |  F_DEC F_IDENTIFIER ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = DEC;
      node->string = $2;
      $$ = node;
   }

   |  F_RETURN '(' expression ')' ';' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = RETURN;
      node->list.next = $3;
      $$ = node;
   }

   |  F_GOTO F_IDENTIFIER ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = GOTO;
      node->string = $2;
      $$ = node;
   }
   ;

expression:

   '(' expression ')' { $$ = $2; }

   | '!' expression {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = NOT;
      node->inner = $2;
      $$ = node;
   }

   | '-' expression {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = MULTIPLY;
      node->factors.left = $2;

      ASTNode* negative = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!negative)) malloc_err();
      negative->type = NUMBER;
      negative->integer = -1; // Multiply by -1.

      node->factors.right = negative;
      $$ = node;
   }

   |  expression '+' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = ADD;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '-' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = SUBTRACT;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '*' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = MULTIPLY;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '/' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = DIVIDE;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression F_GTEQ expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = GTEQ;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression F_LTEQ expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = LTEQ;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '>' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = GREATER;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '<' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = LESS;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression F_EQ expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = EQUALS;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression F_NEQ expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = NEQUALS;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  F_IDENTIFIER '(' declaration ')' ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = FUNCTION_CALL;
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;    
   }
      
   |  F_IDENTIFIER F_INC {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = INC;
      node->string = $1;
      $$ = node;
   }
   |  F_INC F_IDENTIFIER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = INC;
      node->string = $2;
      $$ = node;
   }
      
   |  F_IDENTIFIER F_DEC {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = DEC;
      node->string = $1;
      $$ = node;
   }
   |  F_DEC F_IDENTIFIER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = DEC;
      node->string = $2;
      $$ = node;
   }

   |  F_IDENTIFIER array_reference { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = ARRAY_REF;
      node->list.title = $1;
      node->list.next = $2;
      $$ = node;
   }
   
   |  F_IDENTIFIER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = VARIABLE;
      node->string = $1;
      $$ = node;
   }

   |  F_NUMBER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = NUMBER;
      node->integer = $1;
      $$ = node;
   }

   |  F_CHARACTER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = NUMBER;
      node->integer = (int)$1;
      $$ = node;
   }

   ;

declaration:
   /* empty */ {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = STOP;
      $$ = node;
   }
   |  F_IDENTIFIER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = VARIABLE;
      node->list.title = $1;
      node->list.next = malloc(sizeof(ASTNode));
      if (!node->list.next) malloc_err();
      node->list.next->type = STOP;
      $$ = node;
   }
   |  F_IDENTIFIER ',' declaration {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = VARIABLE;
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;
   }
;


else:
   /* empty */ {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = STOP;
      $$ = node;
   }
   | F_ELSE block { $$ = $2; }
   ;

block:
   '{' statement_list '}' { $$ = $2; }
   | statement { $$ = $1; }
   ;

parameters:
   /* empty */ {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = STOP;
      $$ = node;
   }
   | expression {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = STOP;
      $1->successor = node;
      $$ = $1;
   }
   | expression ',' parameters {
      $1->successor = $3;
      $$ = $1;
   }


array_reference:
   /* empty */ {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = STOP;
      $$ = node;
   }
  | '[' expression ']' {
        ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = ARRAY;
      node->list.inner = $2;
      node->list.next = malloc(sizeof(ASTNode));
      if (!node->list.next) malloc_err();
      node->list.next->type = STOP;
      $$ = node;
    }
  | '[' expression ']' array_reference {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (GCC_UNLIKELY(!node)) malloc_err();
      node->type = ARRAY;
      node->list.inner = $2;
      node->list.next = $4;
      $$ = node;
    }
;

%%

void yyerror(const char *s) {
   fprintf(stderr, "Error: %s\n", s);
}


// The reason an ASTNode* might be equal to 0x000000000000002 is because the union WAS
// setting it to 2, but then this integer was read as an ASTNode*.