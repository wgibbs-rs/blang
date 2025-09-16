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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "llvm.h"
#include "context.h"
#include "error.h"
#include "ast.h"
#include "opt.h"

extern int yyparse(void);                       // declare Bison parser function
extern int yy_scan_string(const char *str);     // lex the file for keywords
void parse_arguments(int argc, char **argv);    // parse the arguments provided to BLang
char* read_file(const char *filename);          // Read an input file into a char*.
void print_help();

CompilerContext ctx = (CompilerContext){
   .emitAssembly = false,
   .emitLLVM = false,
   .dumpAST = false,
   .outputFilename = "a.out",
   .optimization = 0,
};

int main(int argc, char *argv[]) {
   parse_arguments(argc, argv);

   yy_scan_string(ctx.sourceText);           // Feed input
   yyparse();                                // Start parsing

   if (ctx.dumpAST) print_ast();

   initialize_llvm();

   generate_llvm_ir();

   optimize();

   if (ctx.emitLLVM) 
      export_ir();
   else if (ctx.emitAssembly) 
      export_asm();
   else 
      export_bin();
   
   return 0;
}


void parse_arguments(int argc, char *argv[]) {
   if (argc == 1)
      print_help();

   for (int i = 1; i < argc; ++i) {
      if (strcmp(argv[i], "-S") == 0) { ctx.emitAssembly = true; }
      else if (strcmp(argv[i], "-emit-llvm") == 0) { ctx.emitLLVM = true; }
      else if (strcmp(argv[i], "-ast-dump") == 0) { ctx.dumpAST = true; }
      
      else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) { print_help(); }

      else if (strcmp(argv[i], "-o") == 0) {
         ctx.outputFilename = argv[i + 1];
         i++;
      }

      else if (strcmp(argv[i], "-O0") == 0)
         ctx.optimization = 0;
      else if (strcmp(argv[i], "-O1") == 0)
         ctx.optimization = 1;
      else if (strcmp(argv[i], "-O2") == 0)
         ctx.optimization = 2;
      else if (strcmp(argv[i], "-O3") == 0)
         ctx.optimization = 3;
      else if (strcmp(argv[i], "-Os") == 0)
         ctx.optimization = 4;
      else if (strcmp(argv[i], "-Oz") == 0)
         ctx.optimization = 5;

      else { 
         if (argv[i][0] == '-') { fatal_error("unknown argument: \'%s\'", argv[i]); }
         else { 
            ctx.inputFile = argv[i];
            ctx.sourceText = read_file(argv[i]); 
         }
      }
   }
}

char* read_file(const char *filename) {
   FILE *f = fopen(filename, "rb");
   if (!f) fatal_error("failed to open file \"%s\"", filename);

   fseek(f, 0, SEEK_END);
   long n = ftell(f);
   rewind(f);

   char *buf = malloc(n + 1);
   if (!buf) fatal_error("failed to allocate memory for \"%s\"", filename);

   fread(buf, 1, n, f);
   buf[n] = '\0';
   fclose(f);
   return buf;
}

void print_help() {
    printf(
      "Usage: blang [options] <source files>\n"
      "\n"
      "Blang Compiler " BLANG_VERSION_STRING "\n"
      "A simple compiler for the B programming language.\n"
      "\n"
      "Options:\n"
      "  -h, --help            Show this help message and exit\n"
      // "  -v, --version         Show compiler version\n"
      "  -o <file>             Specify output file name (default: a.out)\n"
      "  -S                    Compile to assembly code only\n"
      "  -emit-llvm           Emit LLVM IR instead of machine code\n"
      "  -dump-ast            Output the abstract syntax tree (AST)\n"
      "  -O0, -O1, -O2, -O3    Optimization level (default: -O0)\n"
      "\n"
      "Examples:\n"
      "  blang main.b            Compile and link main.b to a.out\n"
      "  blang -S main.b         Generate assembly code from main.b\n"
      "  blang -O2 -o prog main.b  Compile main.b with optimization level 2 to prog\n"
    );
    exit(0);
}