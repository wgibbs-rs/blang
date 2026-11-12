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
#include "error.h"
%}

%%

"/*"([^*]*|\*+[^*/])*"\*/"       { /* Block comment */       }
"//".*                           { /* Single-line comment */ }

\'[^\']\' { 
   yylval.integer = (int)yytext[1];
   return F_CHARACTER;
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

"="         {  return '=';       }

"+"         {  return '+';       }
"-"         {  return '-';       }
"/"         {  return '/';       }

"+="        {  return F_PLUSEQ;    }
"-="        {  return F_MINUSEQ;   }
"*="        {  return F_TIMESEQ;   }
"/="        {  return F_DIVEQ;     }

"=="        {  return F_EQ;        }
"!="        {  return F_NEQ;       }
">="        {  return F_GTEQ;      }
"<="        {  return F_LTEQ;      }
">"         {  return '>';       }
"<"         {  return '<';       }

"!"         {  return '!';       }

"&&"        {  return F_AND;       }
"||"        {  return F_OR;        }

"<<"        {  return F_LSHIFT;    }
">>"        {  return F_RSHIFT;    }

"++"        {  return F_INC;       }
"--"        {  return F_DEC;       }

"if"        {  return F_IF;        }
"else"      {  return F_ELSE;      }
"while"     {  return F_WHILE;     }

"goto"      {  return F_GOTO;      }

"return"    {  return F_RETURN;    }

".read"     {  yylval.str = "read"; return F_IDENTIFIER;  }
".write"    {  yylval.str = "write"; return F_IDENTIFIER;  }

"auto"      {  return F_AUTO;      }
"extrn"     {  return F_EXTRN;     }

[0-9]+      {  yylval.integer = atoi(yytext); return F_NUMBER; }

[a-zA-Z_][a-zA-Z0-9_]*    { yylval.str = strdup(yytext); return F_IDENTIFIER; }

[ \t\n\r]

.     { error("unexpected character \"%c\"", yytext[0]); }

%%