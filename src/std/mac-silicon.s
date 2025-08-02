
//
// BLang
// Copyright (c) 2025 William Gibbs

// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

.global putchar
.global getchar
.global open
.global close
.global read
.global write

.section .data
getchar_buffer: .space 1

.section .text

putchar:
    mov x1, x0      // Set X1 to the address of the character
    mov x0, #1      // Number for "stdout"
    mov x2, #1      // Stores one byte
    mov x16, #4     // Unix write system call
    svc #0x80       // Call kernel to output the character
    ret

getchar:
    mov x0, #0                          // Number for "stdin"
    adrp x1, getchar_buffer@PAGE        // X1 points to the allocated memory
    add x1, x1, getchar_buffer@PAGEOFF  
    mov x2, #1                          // Take in 1 character
    mov x16, #4                         // System call for "stdio"
    svc #0x80                           // Call kernel to read the character
    mov x0, [x1]
    ret

open:

close:

read:

write:
