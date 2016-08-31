#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "encode.h"


char * exit_labels[5000];
int labelIndex = 0;
 
void encode(ST_ID id) {
    unsigned int totaldim = 1;
    DIMFLAG dimflag;
    int block;
    int size = 0;
    int alignment = 1;
    TYPETAG tag;
    TYPE array;
    ST_DR record =  st_lookup(id, &block);
    TYPE type = record->u.decl.type; // Type is a pointer to a type rec (?)
    tag = ty_query(type);
    TYPE array_entry = ty_strip_modifier(type);
    TYPE final_entry = type;
    
    while(array_entry) {
        unsigned int dim;
        if(TYARRAY == ty_query(final_entry)) {
            array = ty_query_array(final_entry, &dimflag, &dim);
            totaldim = dim * totaldim;
            //printf("array encode, dim: %d\n", dim);
        }
        if(TYPTR == ty_query(final_entry)) {
            alignment = get_size_basic(TYPTR) * alignment;
            //printf("1if tag == TYPTR: %d\n", alignment);
            break;
        }
        final_entry = array_entry;
        array_entry = ty_strip_modifier(array_entry);
        //printf("array encode, totaldim: %d\n", totaldim);

    }
    if(TYPTR != tag) {
        alignment = get_size_basic(ty_query(final_entry));
    }
    
    size = alignment * totaldim;
    
    b_global_decl(st_get_id_str(id), alignment, size);
    b_skip(size);
}

BOOLEAN is_basic_tag(TYPETAG tag) {
    BOOLEAN r = (tag == TYERROR
           || tag == TYPTR
           || tag == TYENUM
           || tag == TYVOID
           || tag == TYFLOAT
           || tag == TYDOUBLE
           || tag == TYLONGDOUBLE
           || tag == TYSIGNEDLONGINT
           || tag == TYSIGNEDSHORTINT
           || tag == TYSIGNEDINT
           || tag == TYUNSIGNEDLONGINT
           || tag == TYUNSIGNEDSHORTINT
           || tag == TYUNSIGNEDINT
           || tag == TYUNSIGNEDCHAR
           || tag == TYSIGNEDCHAR);
    return r;
}

void encode_var(ST_ID id, int block)
{
  printf("in encode, encode_var\n");
  ST_DATA_REC record =  *st_lookup(id,&block);
  
  TYPE type = record.u.decl.type; 
  TYPETAG typetag = ty_query(type);
  unsigned int alignSize;
  unsigned int dim;
  if(typetag == TYARRAY)
  {
    DIMFLAG dimflag;
    TYPE elemType = ty_query_array(type, &dimflag, &dim);
    //printf("encode_var, in if, dim = %u\n",dim);
    alignSize = get_size_basic(ty_query(elemType));
    //printf("encode_var, in if, alignSize = %u\n",alignSize);
    //void b_global_decl (char *id, int alignment, unsigned int size)
    b_global_decl(st_get_id_str(id),(int)alignSize,dim*alignSize);
    
  }else
  {
    alignSize = get_size_basic(typetag);
    char *idPtr = st_get_id_str(id);
    b_global_decl(idPtr,(int)alignSize,alignSize);
  }
}

void encode_expr(EXPR expr) {
 // printf("in encode, encode_expr \n");
  if(expr==NULL) {
    msg("encode expr no tag");
    return;
  }
  if(expr->tag == GDECL_EXPR) {
    b_push_ext_addr(st_get_id_str(expr->u.global_id));
  } else if(expr->tag == BINOP_EXPR) {
    encode_binop(expr);
  } else if(expr->tag == UNOP_EXPR) {
    encode_unop(expr);
  } else if(expr->tag == FUNC_EXPR) {
    encode_func_expr(expr);
  } else {
    encode_simple_expr(expr);
  }
  /*else
  {
  	printf("encode: Not binop or unop \n");
  }*/
}

/*****                                *****
 ***** Expression evaluation routines *****
 *****                                *****/

/* Pop a datum off the control stack (used in expression evaluation)

#define  b_pop()  b_internal_pop(TRUE)

   Not for external use; use b_pop() instead.
   Removes and discards a value from the top of the stack.  If passed TRUE,
   a comment is placed in the assembly code.  This function is used, for
   example, to discard the return value in an assignment statement or
   function call

void b_internal_pop (BOOLEAN display_flag);


/***** Nullary operators (zero items popped) *****/
/*
   b_duplicate pushes a duplicate of the datum currently on the stack.
   The datum is assumed to be of the given type.

void b_duplicate (TYPETAG type);

   b_push_ext_addr accepts a global variable name and emits 
   code to push the address of that variable onto the stack.

void b_push_ext_addr (char *id);

 Added to unify assignment of local and global variables. -SF 2/3/96 
  b_push_loc_addr accepts an offset value (from the frame pointer)
   as parameter, and emits code to push the effective address offset(%ebp)
   onto the stack.  This is what you would call to get the actual address
   of a parameter or local variable onto the stack, given the offset
   value for the variable.

void b_push_loc_addr (int offset);

 b_push_const_int accepts an integer value and emits code to
   push that value onto the stack.

void b_push_const_int (int value);

 b_push_const_double accepts a double value and emits code to
   push that value onto the stack.  It does this by generating a new 
   label for the value, inserting a labeled double value (two .long's)
   into the .rodata section, and pushing the 8-byte value at the label 
   onto the stack.

void b_push_const_double (double value);

 b_push_const_string accepts a string and emits code to "push
   the string onto the stack."  It does this by generating a new 
   label for the string, inserting a labeled line of ascii text 
   into the .rodata section, and pushing the address of the label 
   onto the stack.

void b_push_const_string (char *string);
*/
void encode_simple_expr(EXPR expr)
{
  TYPE type = expr->type; 
  TYPETAG typetag = ty_query(type);

  if(typetag == TYUNSIGNEDCHAR)
  {
    b_push_const_string(expr->u.val_str);
  }
  else if(typetag == TYSIGNEDSHORTINT)
  {
  	b_push_const_int(expr->u.val_int);
  }
  else if(typetag == TYUNSIGNEDINT)
  {
  	b_push_const_int(expr->u.val_int);
  }
  else if(typetag == TYSIGNEDLONGINT)
  {
    b_push_const_int(expr->u.val_dbl);
  }
  else if(typetag == TYSIGNEDINT)
  {
    b_push_const_int(expr->u.val_dbl);
  }
  else if(typetag == TYFLOAT)
  {
    b_push_const_double(expr->u.val_dbl);
  }
  else if(typetag == TYDOUBLE)
  {
    b_push_const_double(expr->u.val_dbl);
  }
}


//{PLUS, MINUS, TIMES, DIV, MOD, ASSIGN, EQUALS, LESS_THAN, LESS_EQUALS, 
             //NOT_EQUALS, GREATER_THAN, GREATER_EQUALS} BINOP_TYPE;

/***** Binary operators (two items popped) *****/

/* b_assign accepts a type and emits code to store a value of that
   type in a variable OF THE SAME TYPE.  It assumes that a value of 
   that type is at the top of the stack and that the address of the 
   variable is the next item on the stack.  It pops both items off 
   the stack, stores the value at the address, AND PUSHES THE VALUE
   BACK ONTO THE STACK.  In other words, this is the code you would need 
   for assigning a value to a variable in C.  Note that it is assumed that
   the stack contains the actual address of the object, so for local
   variables and parameters, you must obtain the actual address beforehand
   using b_push_loc_addr().  To do an an assignment in Pascal, which does
   not use the value, follow b_assign() with b_pop().
*/

/* b_arith_rel_op accepts a binary arithmetic or relational operator
   and a type.  The operators are:

    B_ADD       add (+)
	B_SUB       substract (-) 
	B_MULT      multiply (*)
	B_DIV       divide (/)
	B_MOD       mod (%)
	B_LT        less than (<)
	B_LE        less than or equal to (<=)
	B_GT        greater than (>)
	B_GE        greater than or equal to (>=)
	B_EQ        equal (==)
	B_NE        not equal (!=)
   
   It assumes that two values of the indicated type are on the 
   stack.  It pops those values off the stack, performs the 
   indicated operation, and pushes the resulting value onto
   the stack.

   No arithmetic on pointers is allowed in this function,
   although pointer comparisons are okay.  For pointer arithmetic,
   use b_ptr_arith_op.

   NOTE:  For arithmetic operators that are not commutative, it
          assumes that the operands were pushed onto the stack
	  in left-to-right order (e.g. if the expression is
	  x - y, y is at the top of the stack and x is the 
	  next item below it.

   NOTE:  For relational operators, a value of either 1 (true)
          or 0 (false) is pushed onto the stack.
*/
void encode_binop(EXPR expr) {
  TYPETAG type = ty_query(expr->type);
  TYPETAG left_type_tag, right_type_tag;
  
  //make sure operands are encoded first (could be long subtrees)
  encode_expr(expr->u.binop.left_arg);
  encode_expr(expr->u.binop.right_arg);

  left_type_tag  = ty_query(expr->u.binop.left_arg->type);
  right_type_tag = ty_query(expr->u.binop.right_arg->type);

  if(expr->u.binop.op == PLUS) {
    b_arith_rel_op (B_ADD, type);
  } else if(expr->u.binop.op == MINUS) {
  	b_arith_rel_op (B_SUB, type);
  } else if(expr->u.binop.op == TIMES) {
    b_arith_rel_op (B_MULT, type);
  } else if(expr->u.binop.op == MOD) {
    b_arith_rel_op (B_MOD, type);
  } else if(expr->u.binop.op == DIV) {
  	b_arith_rel_op (B_DIV, type);
  } else if(expr->u.binop.op == ASSIGN) {
    //may be doing this too soon. 
    //if left expr is lvar, push it's address
    if (expr->u.binop.left_arg->tag == LDECL_EXPR) {
      b_push_loc_addr(-8);
    }
    //Maybe conversion idk
   //checking the left and right args types and then converting if necessary
  if(left_type_tag == TYDOUBLE && left_type_tag != right_type_tag) {
	b_convert(right_type_tag, left_type_tag);
  }
    b_assign(left_type_tag);
   // b_pop();
  } else if(expr->u.binop.op == LESS_THAN) {
    b_arith_rel_op (B_LT, type);
  } else if(expr->u.binop.op == LESS_EQUALS) {
  	b_arith_rel_op (B_LE, type);
  } else if(expr->u.binop.op == GREATER_THAN) {
    b_arith_rel_op (B_GT, type);
  } else if(expr->u.binop.op == GREATER_EQUALS) {
    b_arith_rel_op (B_GE, type);
  } else if(expr->u.binop.op == EQUALS) {
  	b_arith_rel_op (B_EQ, type);
  } else if(expr->u.binop.op == NOT_EQUALS) {
    b_arith_rel_op (B_NE, type);
  }
}


/***** Unary operators (one item popped) *****/
/*
   b_deref accepts a type.  It assumes that the address of
   a variable of that type is on the stack.  It pops the
   address and pushes the value stored at that address
   onto the stack.
void b_deref (TYPETAG type);

   b_convert accepts a from_type and a to_type and emits code to
   convert a value of type from_type to a value of type to_type.
   It assumes that there is a value of type from_type on the 
   stack.  That value is popped off the stack, converted to a value 
   of the to_type, and pushed back onto the stack.
void b_convert (TYPETAG from_type, TYPETAG to_type);

   b_negate accepts a type and emits code to negate a value of 
   that type.  It assumes a value of that type is on the stack.
   It pops that value off the stack, negates it, and pushes it
   back onto the stack.
void b_negate (TYPETAG type);

   Changed to treat uniformly global and local variables, and to
   allow arbitrary l-values (not just id's).  -SF 2/3/96 
   b_inc_dec accepts a type, an increment-decrement operator
   (B_PRE_INC, B_POST_INC, B_PRE_DEC, B_POST_DEC), and a size
   parameter.  It emits code to do the indicated
   increment-decrement operation on a variable of the indicated
   type.  It is assumed that a pointer (l-value) is on top of
   the stack.  The function emits code to pop the pointer off the
   stack, increment/decrement the variable pointed to (which is assumed
   to have the given type), then pushes the value (r-value!) of the
   variable back on the stack.  The value pushed is that of the variable
   either before or after the inc/dec, depending on which operator
   was used.

   The size parameter is ignored unless type is TYPTR, in which case
   size should be the size (in bytes) of a datum pointed to by a
   pointer of this type.

void b_inc_dec (TYPETAG type, B_INC_DEC_OP idop, unsigned int size);
*/
void encode_unop(EXPR expr)
{
  TYPE type = expr->type; 
  TYPETAG typetag = ty_query(type);
  TYPETAG othertag = ty_query(expr->u.unop.arg->type);
  //make sure subtree is encoded
  encode_expr(expr->u.unop.arg);
  if(expr->u.unop.op == DEREF)
  {
    b_deref(typetag);
  }
  else if(expr->u.unop.op == REF)
  {
    b_deref(typetag);
  }
  else if(expr->u.unop.op == CONVERT) {
    msg("test");
    b_convert(othertag, typetag);
  }
  else if(expr->u.unop.op == PTR)
  {
    b_deref(typetag);
  }
  else if(expr->u.unop.op == UMINUS)
  {
    b_negate(othertag);
  }
  else if(expr->u.unop.op == INC)
  {
   b_inc_dec(typetag, B_POST_INC, 8);
  }
  else if(expr->u.unop.op == DEC)
  {
    b_inc_dec (typetag, B_POST_DEC, 8);
  }
}


void encode_func_expr(EXPR expr) {
  //doesnt handle arguments
  char *func_name;
  TYPE return_type;
  PARAM_LIST params;
  PARAMSTYLE paramstyle;
  EXPR func = expr->u.fcall.function;
  
  int size; // param list size
  TYPETAG arg_tag;
  
  //fill in helper vars
  return_type = expr->type;
  size = 0;

  //save function name
  if (func->tag == GDECL_EXPR) {
    func_name = st_get_id_str(func->u.global_id); //not enrolled in symbol
  }
  
  b_alloc_arglist(size);

  //encode argument

  //probably conversions again maybe?
  b_funcall_by_name(func_name, ty_query(return_type));
  //b_pop();
}


//float->double (8 bytes)
//any int thats shorter than int->int (4 bytes)
//char->int (4 bytes)
//could be lazy and mult each piece by 8, because that would be inefficient but work
//important function b_alloc_arglist, b_load_arg, b_funcall_by_name

void encode_init(TYPETAG tag,initArg val)
{
  //void b_alloc_char (int init);
  //void b_alloc_short (int init);
  //void b_alloc_int (int init);
  //void b_alloc_long (long init);
  //void b_alloc_ptr (char *init);
  //void b_alloc_float (double init);
  //void b_alloc_double (double init);
  //TYVOID, TYFLOAT, TYDOUBLE, TYLONGDOUBLE, TYSIGNEDLONGINT,
      //TYSIGNEDSHORTINT, TYSIGNEDINT, TYUNSIGNEDLONGINT,
      //TYUNSIGNEDSHORTINT, TYUNSIGNEDINT, TYUNSIGNEDCHAR,
      //TYSIGNEDCHAR, TYSTRUCT, TYUNION, TYENUM, TYARRAY, TYSET,
      //TYFUNC, TYPTR, TYBITFIELD, TYSUBRANGE, TYERROR
  int junk = 9;
  if(tag == TYUNSIGNEDCHAR)
  {
    b_alloc_char(val.intInit);
  }else if(tag == TYSIGNEDSHORTINT)
  {
    b_alloc_short(val.intInit);
  }else if(tag == TYSIGNEDINT)
  {
    b_alloc_int(val.intInit);
  }else if(tag == TYSIGNEDLONGINT)
  {
    b_alloc_long(val.longInit);
  }else if(tag == TYPTR)
  {
    b_alloc_ptr(val.pointerInit);
    //b_alloc_ptr((char*)&junk);
  }else if(tag == TYFLOAT)
  {
    b_alloc_float(val.doubleInit);
  }else if(tag == TYDOUBLE)
  {
    b_alloc_double(val.doubleInit);
  }
}
void encode_init_array(TYPETAG elemType,int arraySize, initArg *elemArray) // RO, use for initializing arrays. elemArray represents e.g. {1.1,2.4,5.9}
{ 
    
  int i;
  for(i = 0; i < arraySize; i++)
  {
    fprintf(stderr,"calling encode_init, i = %d\n",i);
    //encode_init(elemType,elemArray[i]);
    //void b_alloc_gdata(TYPETAG tag, ...)
    b_alloc_gdata(elemType,elemArray[i]);
  }
} 
void encode_test_init(void)
{
  int junk1 = 8;
  printf("encode_test_init, got here\n");
  encode_init(TYUNSIGNEDCHAR,(initArg)'a');
  encode_init(TYSIGNEDSHORTINT,(initArg)5);
  encode_init(TYSIGNEDINT,(initArg)10);
  encode_init(TYSIGNEDLONGINT,(initArg)20);
  encode_init(TYPTR,(initArg)(char*)&junk1);
  //printf("ptr = %ld\n",(long)(char*)&junk1);
  encode_init(TYFLOAT,(initArg)0.0);
  encode_init(TYDOUBLE,(initArg)0.0);/*
  encode_init(TYSIGNEDINT,0.0);*/ 
}
void encode_test_init_array(void)
{
  printf("encode_test_init_array, got here\n");
  int arraySize = 3;
  initArg elemArray[arraySize];
  //int vals[1000] = {3,4,2};
  //long vals[1000] = {444444444,5555555555,2222222222};
  //char vals[1000] = {'a','b','c'}; // (?) Doesn't work.
  //double vals[1000] = {5.2,6.7,8.3};//variable-sized object may not be initialized
  int a = 1,b=2,c=3;
  char *vals[1000] = {(char*)&a,(char*)&b,(char*)&c};
  int i;
  for(i=0;i<arraySize;i++)
  {
    elemArray[i] = (initArg) vals[i];
  }
  encode_init_array(TYPTR,arraySize,elemArray);//&(elemArray[0]));
}
//void b_alloc_gdata(TYPETAG tag, ...);
/*
  backend support for allocating global variables

  1st arg: the type (TYPETAG)
  2nd arg: the initialization value (dependent on type)
*/
//TYPE ty_query_array(TYPE type, DIMFLAG *dimflag, unsigned int *dim);
    /* (C/C++ only)
    This routine takes 1 input parameter which must be an array TYPE.
    It returns through the parameter list the flag that indicates whether
    a dimension is present. Also through the parameter list it returns the
    dimension. Through the function return value it returns the type of
    the array elements.
    */

void add_label(char * label) {
  exit_labels[labelIndex] = (char*)malloc(sizeof(label));
  strcpy(exit_labels[labelIndex], label);
  labelIndex++;
 // msg("add exit label: %s", label);
}

char * get_exit_label() {
  char * label;
  
  if (labelIndex == 0) {
    error("Break not inside loop or switch statement");
  } else {
    label = (char*)malloc(sizeof(exit_labels[labelIndex-1]));
    strcpy(label, exit_labels[labelIndex-1]);
    labelIndex--;
  }
  return label;
}

/*Kelly- Switch - Project Three*/
void encode_switch(EXPR expr, CASE_NODE statement)
{
  //This is going to be an issue when the case is default?
  int initial_value = expr->u.val_int;
  int case_value = statement->c;

  //check here to make sure that the cases match and then execute the one that matches
 /* if(){

  }

  //check for default and do that instead?
  else if(){}*/
}