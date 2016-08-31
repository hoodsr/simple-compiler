#include "defs.h"
#include "types.h"
#include "symtab.h"

typedef enum {DECL_PTR, DECL_REF, DECL_ARRAY, DECL_ID, DECL_FUNC} DECL_TYPE;
typedef enum {INT_EXPR, DOUBLE_EXPR, STR_EXPR, UNOP_EXPR, BINOP_EXPR, 
              ERROR_EXPR, GDECL_EXPR, LDECL_EXPR, FDECL_EXPR, FUNC_EXPR} EXPR_TYPE;
typedef enum {CONVERT, DEREF, REF, PTR, UPLUS, UMINUS, INC, DEC, HASH} UNOP_TYPE;
typedef enum {PLUS, MINUS, TIMES, DIV, MOD, ASSIGN, EQUALS, LESS_THAN, LESS_EQUALS, 
              NOT_EQUALS, GREATER_THAN, GREATER_EQUALS} BINOP_TYPE;

typedef struct tn {
  EXPR_TYPE tag;
  TYPE type;
  union {
    int val_int;
    double val_dbl;
    char * val_str;
    ST_ID global_id; /* For global variables and global functions */

    struct {            
      BOOLEAN is_ref; 
      int link_count; /* Number of ref links to follow to find the var */
    } local_var; // local var and params
    struct {            
      char * global_name; /* The assembler entry point label */
      int link_count; /* Number of ref links to follow to find the fcn */
    } local_func;
    struct {
      UNOP_TYPE op; 
      struct tn *arg; 
      int ln;
    } unop; 
    struct {
      BINOP_TYPE op; 
      struct tn *left_arg, *right_arg;
    } binop;
    struct {
      struct tn *function;
    } fcall;
  } u;
} TREENODE, *EXPR;

//Kepp track of stuff in declarators (*a[2]), could actually probably be rewritten to use expr
typedef struct typen {
  DECL_TYPE dtype;
  struct typen *next, *prev;
  union {
    struct {
      ST_ID id;
    } identifier;
    struct {
      int dim;
    } array;
    struct {
      BOOLEAN is_ref;
    } ptr;
    struct {
      PARAM_LIST param;
    } func;
  } u;
} TYPENODE, *DTYPE_LIST;

//List of declarators: int x, y;
typedef struct idn {
    DTYPE_LIST dlist;
    struct idn *next, *prev;
} IDNODE, *ID_LIST;

//List of declarators: int x, y;
typedef struct casen {
    int c;
    EXPR e;
} CASENODE, *CASE_NODE;


ID_LIST make_id_node(DTYPE_LIST dlist);
ID_LIST append_id(DTYPE_LIST dlist, ID_LIST list);

DTYPE_LIST make_dtype_node(DECL_TYPE dtype, ST_ID id, int dim, PARAM_LIST param_list);
DTYPE_LIST dl_get_tail(DTYPE_LIST list);
DTYPE_LIST dl_get_head(DTYPE_LIST list);
DTYPE_LIST prepend_dtype(DECL_TYPE dtype, DTYPE_LIST dlist, ST_ID id, int dim, PARAM_LIST param_list);

PARAM_LIST make_param_node(DTYPE_LIST dlist, TYPE type);
PARAM_LIST append_param(PARAM_LIST list1, PARAM_LIST list2);
PARAM_LIST param_get_head(PARAM_LIST list);
PARAM_LIST param_get_tail(PARAM_LIST list);
long param_count(PARAM_LIST list);
TYPE build_type(DTYPE_LIST dlist, TYPE type);

void make_variable(TYPE type, ID_LIST list);
TYPE make_array(TYPE type, int dim);
TYPE make_unary_expr(TYPE type, UNOP_TYPE op);

void install_dr(TYPE type, ST_ID id);

//PROJECT 2
EXPR make_int_expr(int value);
EXPR make_id_expr(ST_ID id);
EXPR make_dbl_expr(double value);
EXPR make_unop_expr(UNOP_TYPE op_type, EXPR first_expr);
EXPR make_binop_expr(BINOP_TYPE op_type, EXPR first_expr, EXPR second_expr);
void install_func(ST_ID id, TYPE type);
EXPR make_convert_expr(EXPR sub_expr, TYPE type);

//PROJECT 3
CASE_NODE make_case_statement(int case_value, EXPR expr);