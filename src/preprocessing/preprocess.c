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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "preprocess.h"
#include "../processing/process.h"
#include "../error.h"
#include "../globals.h"


static void print_help();

void read_arguments(int argc, char **argv) {


    // Create a default title for the output, in case no -o flag is passed.
    output_title = malloc(5);
    strcpy(output_title, "a.out");



    if (output_title == NULL) { fatal_error("unable to allocate memory for default output file title \"a.out\""); }

    if (argc == 1) {
        print_help();
        exit(0);
    } else  {
        
        for (int i = 1; i < argc; i++) {

            if ( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
                print_help();
                exit(0);
            }

            if (argv[i][0] == '-') {

                if (strlen(argv[i]) == 2) {
                    switch (argv[i][1])
                    {
                        case 'S':
                            return_asm = 1;
                            break;
                        case 'o':
                            if (i == argc - 1 || argv[i + 1][0] == '-') { fatal_error("argument to '-o' is missing (expected 1 value)"); }
                            else {
                                free(output_title);
                                output_title = malloc( strlen(argv[i + 1]) );
                                output_title = argv[i + 1];
                                i++;
                            }
                            break;
                        default:
                            error("unrecognized flag \"%s\"", argv[i]);
                            break;
                    }
                }

            } else { 
                char *contents = read_file(argv[i]);
                concatenate_pair(&text, contents);
            }
        }
    }
}


static void print_help() {
    printf(
        "bcc 0.01 ( www.github.com/wgibbs-rs/bcc )\n"
        "OUTPUT:\n"
            "\tCan compile B script into assembly and binary files. By default, outputs an executable.\n"
            "\t-S: Return final assembly program; does not create an executable.\n"
        "EXAMPLES:\n"
            "\tbcc --help\n"
            "\tbcc -S main.b\n"
            "\tbcc main.b extra.b\n"
    );

}



void concatenate_pair(char** a, char* b) {
    if (*a == NULL) {
        *a = malloc(strlen(b) + 1);
        if (*a == NULL) fatal_error("malloc failed while concatonating files");
        strcpy(*a, b);
    } else {
        char* temp = realloc(*a, strlen(*a) + strlen(b) + 1);
        if (temp == NULL) fatal_error("realloc failed while concatonating files");
        *a = temp;
        strcat(*a, b);
    }
    free(b);
}



char* read_file(char* name) {
    FILE *file = fopen(name, "r");
    if (file == NULL) { fatal_error("failed to open file \"%s\"", name); }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *buffer = malloc(file_size + 1); // +1 for null-terminator
    if (buffer == NULL) {
        fclose(file);
        fatal_error("failed to allocate memory while reading file \"%s\"", name);
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0'; // Null-terminate for safe string use

    return buffer;
}