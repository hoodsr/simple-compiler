// Ralph Oversteet
 
#include "defs.h"
#include "types.h"
#include "symtab.h"
#include "backend-x86.h"
#include "tree.h"

typedef union init
{
  int intInit;
  long longInit;
  char* pointerInit;
  double doubleInit;
  
}initArg;

void encode_init(TYPETAG tag,initArg val);  // Used for initializing variables. 
void encode_test_init(void);      // Test the variable initializer.
void encode_init_array(TYPETAG elemType,int arraySize,initArg *elemArray); // RO, use for initializing arrays.
void encode_test_init_array(void);  // Test the array initializer.

void encode(ST_ID id);
BOOLEAN is_basic_tag(TYPETAG tag);

void encode_expr(EXPR expr);
void encode_simple_expr(EXPR expr);
void encode_binop(EXPR expr);
void encode_unop(EXPR expr);
void encode_func_expr(EXPR func);

void add_label(char * label);
char * get_exit_label();