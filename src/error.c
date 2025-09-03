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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "error.h"

#if defined(__GNUC__) || defined(__clang__)
__attribute__((cold))
#endif
void error(const char *text, ...) {
   va_list args;
   va_start(args, text);
   printf("bcc: " RED "error: " RESET);
   vprintf(text, args);
   printf("\n");
   va_end(args);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noreturn, cold))
#endif
void fatal_error(const char *text, ...) {
   va_list args;
   va_start(args, text);
   printf("bcc: " RED "error: " RESET);
   vprintf(text, args);
   va_end(args);
   printf("\nbcc: " RED "error: " RESET "compilation failed\n");
   exit(1);
}