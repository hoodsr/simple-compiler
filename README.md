# simple-compiler
A compiler for basic C code written in C, flex and bison.
This code was written in four parts as a school project, described below. Files [file names here] are not included.
https://cse.sc.edu/~fenner/csce531

## Testing
 Run
    ./pcc3 < T2Lxxx_ok.c > T2Lxxx_ok.s

 where 'xxx' is the level number. Then run
    gcc -m32 T2Lxxx_ok.s libxxx.c

each time you want to execute your program, or run
    gcc -m32 -c libxxx.c

once, then run
    gcc -m32 T2Lxxx_ok.s libxxx.o

on subsequent tests on the same level. Note that there is no lib100.c, as none is needed. Finally, run
    ./a.out


## Part 1
###Process C global variable declarations.

This involves both installing the declarations into the symbol table and allocating memory for the variables in the assembly language output file. Also, after all declarations have been processed, you should dump the symbol table (using st_dump() from symtab.h); to do this, run your executable with the "-d" or "--dump" option as a command line argument.

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


## Part 2
###Process C function definitions and C expression statements

To receive 80% of the credit: You must be able to handle function definitions. This includes determining the function return type (you are only responsible for handling functions that return int, char, float, and double), installing the function in the symbol table, generating the necessary assembly code to enter and exit the function, and generating the assembly code for the function body (the statements inside the function). At this level you do not need to handle parameter or local variable declarations.

For this assignment the only legal statement inside a function is an `expression_statement'. For the 80% level of this assignment, expression statements can only be made up of: the assignment operator (=), the basic arithmetic binary operators (+,-,*,/), the unary minus arithmetic operator (-), unsigned integer constants, unsigned floating-point constants, the basic comparison operators (==,<,>,!=,<=,>=), global variables (only types int, char, float, or double), parentheses, and function calls to functions that have no parameters. Please note: the result type of an arithmetic operator (+,-,*,/) is the same as the common type of its (binary converted) operands, but the result type of a comparison operator (==,<,>,!=,<=,>=) is always int, regardless of the type of the operands.

To receive full credit at the 80% level, you should also perform "constant folding" as the expression trees are constructed. Any operator that is detected as having constant operands should be evaluated at compile time (the subtree rooted at the operator is replaced by a constant tree node containing the value of the result).

You are responsible for doing the appropriate C type checking of these expression statements and determining the necessary type conversions. Be sure you understand the C type conversion rules. Implement the "traditional" C rules for type conversions. See the section "C Conversions" below.

For compatibility with legacy code, C (but not C99) allows a function call involving an undeclared function name (it is presumably in another file, or appears later in the current file), and just assumes the function will return a value of type int. To simplify matters, we will adopt the C99 convention and not allow this: every function name appearing in a function call must already be declared or defined (in the same compilation unit). However, you should assume that a function definition without a return type specification declares a function that returns int.

You are also responsible for doing the appropriate semantic error checking. For example, the use of an undeclared identifier in an expression is illegal, even if it is the function name in a function call.

You need not issue an error message for an expression statement that has no side effects. That is, this statement may be processed without a message:

    x+6;

Above, the value of the expression "x+6" is simply ignored. Since we are not yet implementing the return statement, you do not need to check that the proper type is being returned by a function being defined.

To receive 90% of the credit: In addition to obtaining the 80% level, you must add parameter declarations, parameter references in expressions, and function calls that include parameter lists, including reference parameters. You are not responsible for `old style' definitions of parameters, for example,

int f(a, b, c)
char *a;
int c;
double b;
{ ... }

Be sure to check that each parameter has a type declaration (the order of the parameter type declarations is not important). Do not allow any parameter type to be specified by default. You are not responsible for detecting parameter number or parameter type mismatches between the definition of a function and a call to that function. You only need to handle parameters of type int, char, float, double, and pointer types (also allow string literals to be passed as parameters). Be sure you understand what it means for a parameter to be of type char or float (Harbison and Steele, Section 9.4).

You will not be calling any functions that have reference parameters (so all arguments to a function call are r-values), but you need to support reference parameters in functions that you define.

To receive 100% credit: In addition to obtaining the 90% level, you must fully implement compound statements and you must add the increment (++) and decrement(--) operators to your repertoire of arithmetic operators. Implementing compound statements means you must support local variable declarations and local variable references within expression statements. Since compound statements can be nested, this means you must be able to implement C's block structured scope rules. You only need to handle local variable declarations of type int, char, float, and double.

At all levels you are responsible for detecting the relevant duplicate declarations.

The appropriate back end routines to generate x86 assembly code for this assignment are discussed in class and described in backend-x86.h.

You may assume there will not be any initializers in the source code. You are also not responsible for processing storage class specifiers on any declaration.

As in Project Part I: your compiler should be capable of detecting multiple semantic errors in one file, and you may allow the compiler to stop processing with the first syntax error.

C Conversions

There are four kinds of conversions in C: these are called the usual conversions (Harbinson & Steele, C: A Reference Manual (5th ed.), Section 6.3):

Unary conversions. Applied to operands of an operation. The purpose is to reduce the large number of arithmetic types to a smaller number that can be handled by the operators.
Binary conversions. Applied to an operand of a binary operator, based on the type of the other operand. The purpose is to uniformize the types of operands of the binary operator.
Assignment conversions. Applied to the right-hand side of an assignment expression to convert to the type of the left-hand side.
Function argument conversions. Conversions applied to actual arguments in a function call. As with unary conversions, the purpose is to reduce the number of possible types used in the argument list of the function call.
Casting conversions. These are conversions made by an explicit typecast. We will not deal with these kinds of conversions.
We will use the conversion rules for Traditional C rather than Standard C. The rules for the various usual conversions relevant to the project are as follows (here, T represents any data type):

Unary conversions. "float" to "double", "array of T" to "pointer to T", "function returning T" to "pointer to function returning T" , "char" to "int".
Binary conversions. "int" to "double" if the other operand is of type "double".
Assignment conversions. Converts the type of the right-hand side into the type of the left-hand side. Any arithmetic type can be converted to any arithmetic type (this includes "char", "int", "float", and "double"). The constant 0 can be converted to any pointer type.
Function argument conversions. By default (when there is no function prototype), these are the same as the unary conversions (for Traditional C).
Always use the default function argument conversions, even if the function has a prototype. You are not required to check the number and types of actual arguments against the number and types of the corresponding formal parameters (again, even if the function has a prototype).

When a unary operator is applied in an expression and that operator expects an r-value, the r-value of its operand is unary-converted before the operator is applied. When a binary operator (expecting two r-values) is applied, each operand is first unary converted, then the operands are then binary converted (which affects only one of the two operands at most).

The relevant back end routine for applying a conversion operator is b_convert().


## Part 3
###Process C control statements

To receive 80% of the credit (90% of the undergraduate credit): You must process if statements, while statements, break statements, and return statements. Be sure to do the appropriate semantic error checking: break cannot appear outside a while, etc. Consult the appropriate C references and your favorite compilers to determine what conversions are applied to the return expressions.

To receive 90% of the credit (100% of the undergraduate credit): You should also process for statements. Be sure to do any appropriate semantic error checking. The break statement should be implemented in the context of the for statement, too.

To receive 100% of the credit (10 points of extra credit for undergraduates): You must also process switch statements. The only kind of case constant expression you need to support is a simple constant (no operators, not even unary minus). You should enforce a rule that the type of the switch expression must be some integer type (it is subject to the usual unary conversions). Be sure to do other appropriate semantic error checking: case constants must be unique within a switch, only one default label for a given switch, etc. The break statement should be implemented in the context of the switch statement, too.

Before starting on switch it is highly recommended that you first familiarize yourself with switch statement semantics. According to Harbison & Steele (5th ed.), Section 8.7, the switch statement is executed as follows:

The control expression (the expression after the keyword switch) is evaluated.
If the value of the control expression is equal to that of the constant expression in some case label belonging to the switch statement, then program control is transferred to the point indicated by that case label as if by a goto statement.
If the value of the control expressions is not equal to any case label, but there is a default label that belongs to the switch statement, then program control is tranferred to the point indicated by that default label.
If the value of the control expression is not equal to any case label and there is no default label, no statement of the body of the switch statement is executed; program control is tranferred to whatever follows the switch statement.
(Above, we say that a label belongs to a switch statement if it appears in the body of the switch statement but is not inside the body of any nested switch statement.) When control is transferred to a case or default label, execution continues through successive statements, ignoring any additional case or default labels that are encountered, until the end of the switch statement is reached or until control is transferred out of the switch statement by a break or return (or goto or continue) statement.
There are four primary functions in the back end that are needed for this assignment: b_jump, b_cond_jump, b_dispatch, and b_encode_return, and two additional supporting functions, b_label and new_symbol.

The function new_symbol returns a fresh, unique, saved string that can be used as a label and jump destination; b_label emits a given string as a label. Each call to new_symbol returns a different string, which is guaranteed not to conflict with any legal C identifier. The function b_jump generates an unconditional jump to a given label; b_cond_jump generates a conditional jump based on the value of the top of the stack (the value is also popped off the stack).

The function b_dispatch is to be used in handling a switch statement. It takes an integer argument n and generates code that compares n with the value on top of the stack (which must be an integer). If the two values are equal, a jump is executed and the value is popped, otherwise no jump occurs and the value is left on the stack. Thus, several calls to b_dispatch can be called in a row, each comparing the top of the stack with a series of different integer constants.

The function b_encode_return generates code to return from a function with an optional return value. If there is an expression following the return keyword, as in

    return x+3;

then the expression's value should be assignment-converted to the return type of the function, and this type should be passed to b_encode_return. The generated assembly code expects the converted value to be on top of the stack. If there is no expression after the return keyword, then TYVOID should be passed to b_encode_return, and the resulting code does not assume any value on top of the stack. (Note: an expressionless return in a function with a nonvoid return type is not a semantic error! The return value is undefined in this case.)

As in Project parts I and II: your compiler should be capable of detecting multiple semantic errors in one file; and you may allow the compiler to stop processing with the first syntax error.
