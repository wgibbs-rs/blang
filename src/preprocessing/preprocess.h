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
#ifndef _preprocess
#define _preprocess

/*
 * Process for preprocessing; two steps.
 * 
 * Step 1: Take arguments and adjust variable for these. read_arguments(int argc, char**argv);
 * 
 * Step 2: Concatenate files. This can be done with simple char* addition. The main file–the file
 * that contains main()–should be the first, then all others after. Unlike languages like C, B
 * does not support multiple files in the same way. including function is also not a thing in B.
 * Instead, we can simply call the function, and concatonate the files together as one.
*/

/** Read the arguments provided at runtime. This include flags, files, and other inputs. */
extern void read_arguments(int argc, char **argv);

/** Read the contents of a file into a char* variable. */
extern char* read_file(char* name);

#endif // _preprocess