/*
   BCC
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

#ifndef _error
#define _error

#include <stdarg.h>
#include <stdio.h>

#define RED "\033[1;31m"
#define RESET "\033[0m"


/** Alert an error in the console; does not quit the program. */
extern void error(const char *text, ...);

/** Alert an error in the console and quit the program. */
extern void fatal_error(const char *text, ...);


#endif // _error