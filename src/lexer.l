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
#include "parser.h"
%}


%%

"/*"([^*]*|\*+[^*/])*"\*/"       { /* Block comment */       }
"//".*                           { /* Single-line comment */ }


\'[^\']\' { 
   yylval.integer = (int)yytext[1];
   return CHARACTER;
}

"["         {  return '[';       }
"]"         {  return ']';       }

"{"         {  return '{';       }
"}"         {  return '}';       }
"("         {  return '(';       }
")"         {  return ')';       }
":"         {  return ':';       }
";"         {  return ';';       }

","         {  return ',';       }

"*"         {  return '*';       }
"&"         {  return '&';       }

"auto"      {  return AUTO;      }
"extrn"     {  return EXTRN;     }

"="         {  return '=';       }

"+"         {  return '+';       }
"-"         {  return '-';       }
"/"         {  return '/';       }


"+="        {  return PLUSEQ;    }
"-="        {  return MINUSEQ;   }
"*="        {  return TIMESEQ;   }
"/="        {  return DIVEQ;     }

"=="        {  return EQ;        }
"!="        {  return NEQ;       }
">="        {  return GTEQ;      }
"<="        {  return LTEQ;      }
">"         {  return '>';       }
"<"         {  return '<';       }

"&&"        {  return AND;       }
"||"        {  return OR;        }

"<<"        {  return LSHIFT;    }
">>"        {  return RSHIFT;    }

"++"        {  return INC;       }
"--"        {  return DEC;       }

"if"        {  return IF;        }
"else"      {  return ELSE;      }
"while"     {  return WHILE;     }

"goto"      {  return GOTO;      }

"return"    {  return RETURN;    }

".read"     {  yylval.str = "read"; return IDENTIFIER;  }
".write"    {  yylval.str = "write"; return IDENTIFIER;  }




[0-9]+      {  yylval.integer = atoi(yytext); return NUMBER; }

[a-zA-Z_][a-zA-Z0-9_]*    { yylval.str = strdup(yytext); return IDENTIFIER; }

[ \t\n\r]


.     { printf("blang: \033[1;31merror:\033[0m unexpected character \"%c\"\n", yytext[0]); }

%%