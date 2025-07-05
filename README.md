# llshader
Compiler frontend for Open Shading Language - A C++ language for writing custom shaders, usable in applications like Blender  

## Components
- Lexer - Parse the code as a string of characters to generate tokens; Identify incorrect tokens
- Parser - Parse the token buffer to generate the abstract syntax tree; Identify syntax errors
- Semantic Analyzer - Traverse the AST to identify semantic errors in the code
- CodeGen - Convert the AST into a series of three address codes, as found in a .oso file
