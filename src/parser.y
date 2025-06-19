%{
#include <stdio.h>

extern void yyerror(const char *s);
extern int yylex(void);
extern int yyparse(void);
extern int yy_scan_string(const char *str);

%}


%union {
   int      integer;
   char*    str;
   char     character;
}


%token <str> IDENTIFIER

%token AUTO EXTRN

%token <integer> NUMBER CHARACTER
%token <str> STRING

%token PLUSEQ MINUSEQ TIMESEQ DIVEQ
%token EQ NEQ AND OR

%token <character> UNKNOWN


%left '+' '-'
%left '*' '/'

%%

program:
      /* empty */
   | program global_declaration
   | program function
   ;

function:
      IDENTIFIER '(' declaration ')' '{' statement_list '}'    { printf("Got a function\n"); }
   ;


statement_list:
   /* empty */
   |  statement_list statement
   ;

statement:
      declaration_list                             { printf("Declaration.\n");            }
   |  IDENTIFIER '=' expression ';'                { printf("Assignment.\n");             }
   ;

expression:
      expression '+' expression                    { printf("Addition.\n");               }
   |  expression '-' expression                    { printf("Subtraction\n");             }
   |  expression '*' expression                    { printf("Multiplication\n");          }
   |  expression '/' expression                    { printf("Division\n");                }
   |  IDENTIFIER
   |  NUMBER
   |  CHARACTER
;


declaration_list:
      AUTO declaration ';'
   ;

declaration:
   |  IDENTIFIER
   |  declaration ',' IDENTIFIER
   ;

global_declaration:
   |  IDENTIFIER expression ';'        { printf("Global declaration.\n");  }
   ;

%%

void yyerror(const char *s) {
   printf(stderr, "Error: %s\n", s);
}