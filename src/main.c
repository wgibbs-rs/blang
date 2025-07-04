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

extern int yyparse(void);  // declare Bison parser function
extern int yy_scan_string(const char *str);  // From Flex

void parse_arguments(int argc, char **argv);
char* read_file(char *file);

void preprocess_text_input(char *text);


CompilerContext ctx = (CompilerContext){
   .emitAssembly = false,
   .emitLLVM = false,
   .outputFilename = "a.out"
};


int main(int argc, char *argv[]) {

   parse_arguments(argc, argv);

   initialize_llvm();

   yy_scan_string(ctx.sourceText);           // Feed input
   yyparse();                                // Start parsing

   print_ast();

   generate_llvm_ir();

   if (!(ctx.emitAssembly || ctx.emitLLVM)) {
      generate_binary();
   }

   return 0;
}


void parse_arguments(int argc, char *argv[]) {

   // Loop through each argument. Start at 1 to skip "./blang"
   for (int i = 1; i < argc; ++i) {

      // If "-S" is passed, the program will return assembly instead of machine code.
      if (strcmp(argv[i], "-S") == 0) { ctx.emitAssembly = true; }

      // If "-emit-llvm" is passed, the program will return LLVM IR instead of machine code.
      else if (strcmp(argv[i], "-emit-llvm") == 0) { ctx.emitLLVM = true; }

      else if (strcmp(argv[i], "-o") == 0) {
         ctx.outputFilename = argv[i + 1];
         i++;
      }

      else { 
         if (argv[i][0] == '-') { fatal_error("unknown argument: \'%s\'\n", argv[i]); }
         else { ctx.sourceText = read_file(argv[i]); }
      }
   }
}

char* read_file(char *filename) {
   FILE* file = fopen(filename, "rb");  // Open in binary mode
   if (!file) {
      fatal_error("failed to open file \"%s\"", filename);
   }

   // Seek to end to get file size
   fseek(file, 0, SEEK_END);
   long length = ftell(file);
   rewind(file);  // Go back to beginning

   // Allocate memory for content + null terminator
   char* buffer = (char*)malloc(length + 1);
   if (!buffer) {
      fclose(file);
      fatal_error("failed to allocate memory for contents of file \"%s\"", filename);
   }

   // Read into buffer
   size_t bytesRead = fread(buffer, 1, length, file);
   buffer[bytesRead] = '\0';  // Null-terminate

   fclose(file);
   return buffer;
}
