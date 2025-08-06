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
#include "./../src/ast.h"

#include "./../src/error.h"

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


%token <str> IDENTIFIER
%token AUTO EXTRN
%token <integer> NUMBER CHARACTER ARRAY
%token <str> STRING
%token PLUSEQ MINUSEQ TIMESEQ DIVEQ INC DEC
%token EQ NEQ GTEQ LTEQ AND OR
%token LSHIFT RSHIFT
%token WHILE IF ELSE
%token GOTO
%token RETURN


%type <node> function 
%type <node> statement_list statement 
%type <node> expression declaration 
%type <node> array_reference else block;

%left '+' '-'
%left '*' '/'

%%

program:
   /* empty */ 
   |  program IDENTIFIER expression ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _GLOBAL_DECLARATION;
      node->list.title = $2;
      node->list.next = $3;
      append_statement(node);
   }
   |  program function { append_statement($2); }
   ;

function:
   IDENTIFIER '(' declaration ')' '{' statement_list '}' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _FUNCTION;
      node->function.title = $1;
      node->function.args = $3;
      node->function.statements = $6;
      $$ = node;
   }
   ;


statement_list:
   /* empty */ {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
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
   AUTO declaration ';' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _AUTO;
      node->list.next = $2;

      // Set all variables defined to be of type "AUTO"
      ASTNode* current = node;
      while (current->type != STOP) {
         current->list.variableType = VAR_AUTO;
         current = current->list.next;
      }

      $$ = node;
   }
   |  EXTRN declaration ';' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _EXTRN;
      node->list.next = $2;

      // Set all variables defined to be of type "EXTRN"
      ASTNode* current = node;
      while (current->type != STOP) {
         current->list.variableType = VAR_EXTRN;
         current = current->list.next;
      }

      $$ = node;
   }
   |  IDENTIFIER '=' expression ';' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _ASSIGNMENT;
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;
   }

   |  WHILE '(' expression ')' block { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _WHILE_LOOP;
      node->list.inner = $3;
      node->list.next = $5;
      $$ = node;
   }
   
   |  IF '(' expression ')' block else { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _IF;
      node->if_t.cond = $3;
      node->if_t.statements = $5;
      node->if_t.else_t = $6;
      $$ = node;
   }

   |  IDENTIFIER ':' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _LABEL;
      node->string = $1;
      $$ = node;
   }

   |  IDENTIFIER '(' declaration ')' ';' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _FUNCTION_CALL;
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;
   }


   |  IDENTIFIER INC ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _INC;
      node->string = $1;
      $$ = node;
   }
   |  INC IDENTIFIER ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _INC;
      node->string = $2;
      $$ = node;
   }
      
   |  IDENTIFIER DEC ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _DEC;
      node->string = $1;
      $$ = node;
   }
   |  DEC IDENTIFIER ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _DEC;
      node->string = $2;
      $$ = node;
   }

   |  RETURN '(' expression ')' ';' { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _RETURN;
      node->list.next = $3;
      $$ = node;
   }

   | GOTO IDENTIFIER ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _GOTO;
      node->string = $2;
      $$ = node;
   }

   ;

expression:

   '(' expression ')' { $$ = $2; }

   | '!' expression {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _NOT;
      node->inner = $2;
      $$ = node;
   }

   |  expression '+' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _ADD;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '-' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _SUBTRACT;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '*' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _MULTIPLY;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '/' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _DIVIDE;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression GTEQ expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _GTEQ;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression LTEQ expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _LTEQ;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '>' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _GREATER;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression '<' expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _LESS;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression EQ expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _EQUALS;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  expression NEQ expression {  
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _NEQUALS;
      node->factors.left = $1;
      node->factors.right = $3;
      $$ = node;
   }

   |  IDENTIFIER '(' declaration ')' ';' {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _FUNCTION_CALL;
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;    
   }
      
   |  IDENTIFIER INC {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _INC;
      node->string = $1;
      $$ = node;
   }
   |  INC IDENTIFIER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _INC;
      node->string = $2;
      $$ = node;
   }
      
   |  IDENTIFIER DEC {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _DEC;
      node->string = $1;
      $$ = node;
   }
   |  DEC IDENTIFIER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _DEC;
      node->string = $2;
      $$ = node;
   }

   |  IDENTIFIER array_reference { 
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _ARRAY_REF;
      node->list.title = $1;
      node->list.next = $2;
      $$ = node;
   }
   
   |  IDENTIFIER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _VARIABLE;
      node->string = $1;
      $$ = node;
   }

   |  NUMBER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _NUMBER;
      node->integer = $1;
      $$ = node;
   }

   |  CHARACTER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _NUMBER;
      node->integer = (int)$1;
      $$ = node;
   }

   ;

declaration:
   /* empty */ {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = STOP;
      $$ = node;
   }
   |  IDENTIFIER {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _VARIABLE;
      node->list.title = $1;
      node->list.next = malloc(sizeof(ASTNode));
      if (!node->list.next) malloc_err();
      node->list.next->type = STOP;
      $$ = node;
   }
   |  IDENTIFIER ',' declaration {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _VARIABLE;
      node->list.title = $1;
      node->list.next = $3;
      $$ = node;
   }
;


else:
   /* empty */ {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = STOP;
      $$ = node;
   }
   | ELSE block { $$ = $2; }
   ;

block:
   '{' statement_list '}' { $$ = $2; }
   | statement { $$ = $1; }
   ;


array_reference:
   /* empty */ {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = STOP;
      $$ = node;
   }
  | '[' expression ']' {
        ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _ARRAY;
      node->list.inner = $2;
      node->list.next = malloc(sizeof(ASTNode));
      if (!node->list.next) malloc_err();
      node->list.next->type = STOP;
      $$ = node;
    }
  | '[' expression ']' array_reference {
      ASTNode* node = malloc(sizeof(ASTNode));
      if (!node) malloc_err();
      node->type = _ARRAY;
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