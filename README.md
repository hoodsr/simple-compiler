# simple-compiler
A compiler for basic C code written in C, flex and bison.
This coed was written in four parts as a school prject, described below. Files [file names here] are not included.

## Part 1
Process C global variable declarations. This involves both installing the declarations into the symbol table and allocating memory for the variables in the assembly language output file. Also, after all declarations have been processed, you should dump the symbol table (using st_dump() from symtab.h); to do this, run your executable with the "-d" or "--dump" option as a command line argument.

Your compiler should read C source code from stdin and write the x86 assembly language output to stdout. Your compiler executable should be called pcc3. You will not have to emit assembly code explicitly, but rather call appropriate routines in the back end (backend-x86.c and backend-x86.h). Besides altering the gram.y file, put syntax tree-building functions into a new file tree.c, with definitions for export in tree.h. Put code-generating routines into a new file encode.c, with definitions for export in encode.h. With few exceptions throughout the project, all backend routines are called from encode.c (some may be called directly from the grammar). No backend routines should be called from tree.c, hence you will not need to include backend-x86.h in tree.c.

The scores given below are for graduate students. Undergraduates get a 10% boost overall.

To receive 80% of the credit: You must be able to process the following basic type specifiers: int, char, float, and double. You may limit the syntax so that only one type specifier may be given per declaration. You must also be able to handle pointer and array type modifiers. You may limit the syntax so that array dimensions must always be given. You may assume the dimension given will always be an unsigned integer constant. Each declaration should include an identifier (id). If not, an error should be issued. A symbol table entry should be made for each id. The entry should indicate the type of the declaration. Routines for building and analyzing types are in the types module (types.h) and bucket module (bucket.h), and routines for manipulating the symbol table are in the symbol table module (symtab.h). You are required to use these modules, but you are not allowed to modify them. For more on these and the other modules, see the Resources section, below.

To receive 90% of the credit: In addition to obtaining the 80% level, you should also allow multiple type specifiers per declaration. You should handle the additional specifiers signed, unsigned, short, and long. You should add the necessary semantic checks and error messages to support multiple type specifiers (e.g., short short, unsigned double, et cetera are illegal). You should also add the function type modifier. You should add the necessary semantic checks and error messages to support function modifiers (it is illegal for a function to return a function, for example). Only "old style" functions need to be supported at this level, that is, with no parameter list between parentheses.

To receive 100% of the credit: In addition to obtaining the 90% level, you should also allow parameters in function declarations. You should insist that each parameter declaration includes an id (else semantic error). The possible parameter types are the same as described in the previous levels, including pointers, arrays, and functions. You should also support the void return type for a function. A parameter may be a reference parameter, e.g.,

    int f(int& a);
    void g(int (&a)[5]);

This is the only aspect of the language that is not part of C. You can assume that any "&" appears only once in a parameter declaration, and only modifies the complete parameter type (so for example, you will never see int h(int&* a);). You can also assume that any parameter of function type has no parameter declarations of its own (you will only see "old style" function types as parameters).

The semantic errors you should check for at this level are that each parameter declaration must include an id, and that the same id should not appear more than once in the same parameter list.

To receive 110% of the credit (that is, 10% extra credit): In addition to obtaining the 100% level, you should also be capable of processing initializers. You may assume that the initializing expressions will only be unsigned constants. You should support initializations of arrays, including multidimensional arrays. For multidimensional arrays "the brace-enclosed list of initializers should match the structure of the variable being initialized" (to quote Harbison & Steele, "C: A Reference Manual"). Arrays may be incompletely initialized; fill remaining slots with zeros. You do not have to support the initialization of arrays with string literals. You also may assume that pointers will only be initialized to zero. Be sure to consider semantic errors: wrong number of initializers, wrong type, etc.

The x86 (actually 32-bit i386) assembly code to be emitted for this assignment is generated automatically by calling functions in backend-x86.c, which I will discuss briefly in class.

At all levels you are responsible for detecting duplicate declarations. At the 100% level, you must also detect duplicate declarations in parameter lists.

Your compiler should be capable of detecting multiple semantic errors in one file. You can make arbitrary decisions about how to proceed when errors occur (for instance, with a duplicate declaration you might decide to ignore the second declaration). The important point is to do something so you can proceed (without causing a later segmentation fault during compilation).

You may allow the compiler to stop processing with the first syntax error. A syntax error is defined with respect to the distributed grammar (gram.y, see next paragraph).
