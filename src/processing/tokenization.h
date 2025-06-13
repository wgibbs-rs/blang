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


/** Defines token types for the B language. */
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_LITERAL,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION
} TokenType;


/** Defines the information associated with a token. */
typedef struct {
    TokenType type;
    const char *value;  // pointer to the original source, or copy into heap
} Token;



Token *generated_tokens;

/** Convert text input into tokens – a step-by-step version of code. */
void tokenize_input_text();
