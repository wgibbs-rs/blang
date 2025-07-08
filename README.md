# BLang
BLang is a compiler for the *B programming language*, a now-extinct programming language developed at Bell Labs in 1969 by Ken Thompson and Dennis Ritchie.

Written in C, BLang aims to be historically accurate to the language, consistently referencing the few reference manuals available online, including "A Tutorial Introduction to the B Language" by B.W.Kernighan, and "Users' Reference to B on MH-TSS" by S.C.Johnson. BLang is created with Flex and Bison, a lexer and parser respectively that can together convert B code into an AST. From there, BLang uses LLVM to generate LLVM Intermediate Representation. Finally, optimizations are passed and executable code is created.

To compile B code with BLang, you can simply run `blang example.b`, which will convert example.b into an executable `example`, given no errors are present. To emit LLVM IR, you can include the flag `-emit-llvm` which will create a file containing the IR in text format. To emit assembly code of the target architecture, you can include the flag `-S`, which will create a file called `example.s` containing the generated assembly code.
