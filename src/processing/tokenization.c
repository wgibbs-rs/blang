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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <ctype.h>

#include "../globals.h"
#include "../error.h"



/** Push a lexeme onto the array. */
static void token_flush(char *t) {
    printf("Added \"%s\"\n", t);
}


static void token_insert(char *t, char c) {
    printf("Inserted character.n\n");
}


/** Convert text input into tokens – a step-by-step version of code. */
void tokenize_input_text() {    

    char lexeme[512];   // Allocate enough space (the size of the raw text).
    int lex_index = 0;

    bool is_string = false;

    bool is_comment = false;

    int text_len = strlen(text);

    // iterate through each character, either adding it to a lexeme, ignoring, or closing.
    for (int i = 0; i < strlen(text); i++) {

        if (is_comment) {

            // Test for end of comment, otherwise continue.
            if (i < text_len - 1 && text[i] == '*' && text[i + 1] == '/') {
                is_comment = false;
                lex_index = 0;
                i++;
            } else { continue; } // Still a comment; continue

        } else if (is_string) {

            switch (text[i]) {

                case '"':
                    push(lexeme, lex_index);
                    lex_index = 0;
                    is_string = false;
                    break;

                case '*':
                    if (text[i] < text_len - 1) {

                        switch (text[i + 1]) {
                            case '0':
                            case 'e':
                                lexeme[lex_index] = '\0';
                                break;
                            case '(':
                                lexeme[lex_index] = '{';
                                break;
                            case ')':
                                lexeme[lex_index] = '}';
                                break;
                            case '<':
                                lexeme[lex_index] = '[';
                                break;
                            case '>':
                                lexeme[lex_index] = ']';
                                break;
                            case 't':
                                lexeme[lex_index] = '\t';
                                break;
                            case '*':
                                lexeme[lex_index] = '*';
                                break;
                            case '\'':
                                lexeme[lex_index] = '\'';
                                break;
                            case '\"':
                                lexeme[lex_index] = '\"';
                                break;
                            case 'n':
                                lexeme[lex_index] = '\n';
                                break;
                            case 'r':
                                lexeme[lex_index] = '\r';
                                break;
                            case 'f':
                                lexeme[lex_index] = '\f';
                                break;
                            case 'b':
                                lexeme[lex_index] = '\b';
                                break;
                            case 'v':
                                lexeme[lex_index] = '\v';
                                break;
                            case 'x':
                                // TODO
                                break;
                            case '#':
                                // TODO
                                break;
                            default:
                                fatal_error("invalid * escape sequence \'*%c\'", text[i + 1]);
                                break;
                        }
                        i++;
                    } 
                    break;
                case "$":
                    if (text[i] < text_len - 1) {

                        switch (text[i + 1]) {
                            case '(':
                                lexeme[lex_index] = '{';
                                break;
                            case ')':
                                lexeme[lex_index] = '}';
                                break;
                            case '<':
                                lexeme[lex_index] = '[';
                                break;
                            case '>':
                                lexeme[lex_index] = ']';
                                break;
                            case '+':
                                lexeme[lex_index] = '|';
                                break;
                            case '-':
                                lexeme[lex_index] = '^';
                                break;
                            case 'a':
                                lexeme[lex_index] = '@';
                                break;
                            case '\'':
                                lexeme[lex_index] = '`';
                                break;
                            default:
                                fatal_error("invalid $ escape sequence \'*%c\'", text[i + 1]);
                                break;
                        }
                        i++;
                        // 01001000 01100101 01101100 01101100 01101111
                    } 
                    break;
                default:
                    if (text[i] >= 0 && text[i] <= 127) {
                        lexeme[lex_index] = text[i];
                    } else {
                        fatal_error("unrecognized character \"%c\"", text[i]);
                    }
                    break;
            }

        } else {    // Text that is not a comment or string.

            // TO DO

        }
    }
}
