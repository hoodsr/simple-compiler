#include "tree.h"
#include <stdlib.h>

//ID_LIST Start
ID_LIST make_id_node(DTYPE_LIST dlist) {
  ID_LIST new_node = (ID_LIST)malloc(sizeof(IDNODE));

  new_node->prev = NULL;
  new_node->next = NULL;
  new_node->dlist = dlist;
  
  return new_node;
}

ID_LIST append_id(DTYPE_LIST dlist, ID_LIST list) {
  ID_LIST new_node = make_id_node(dlist);

  if (list) {    
      new_node->prev = list;
      list->next = new_node;
  }
  
  return new_node;
}

ID_LIST get_head(ID_LIST list) {
  ID_LIST head = list;
  while (head && head->prev) {
    head = head->prev;
  }
  return head;
}
//ID_LIST End

//DTYPE_LIST Start
DTYPE_LIST make_dtype_node(DECL_TYPE dtype, ST_ID id, int dim, PARAM_LIST param_list) {
  DTYPE_LIST new_node = (DTYPE_LIST)malloc(sizeof(TYPENODE));

  new_node->prev = NULL;
  new_node->next = NULL;
  new_node->dtype = dtype;

  if(dtype == DECL_ID) {
    new_node->u.identifier.id = id;
  }
  else if(dtype == DECL_ARRAY) {
    new_node->u.array.dim = dim;
  } else if(dtype == DECL_FUNC) {

    new_node->u.func.param = param_get_head(param_list);
  } else if(dtype == DECL_PTR) {
    new_node->u.ptr.is_ref = FALSE;
  } else if(dtype == DECL_REF) {
    new_node->u.ptr.is_ref = TRUE;
  }
  return new_node;
}

DTYPE_LIST dl_get_tail(DTYPE_LIST list) {
  DTYPE_LIST tail = list;
  while (tail && tail->next) {
    tail = tail->next;
  }
  return tail;
}

DTYPE_LIST dl_get_head(DTYPE_LIST list) {
  DTYPE_LIST head = list;
  while (head && head->prev) {
    head = head->prev;
  }
  return head;
}

DTYPE_LIST prepend_dtype(DECL_TYPE dtype, DTYPE_LIST dlist, ST_ID id, int dim, PARAM_LIST param_list) {
  DTYPE_LIST new_node = make_dtype_node(dtype, id, dim, param_list);

  if(dlist) {
    new_node->next = dlist;
    dlist->prev = new_node;
  }
  return new_node;
}
//DTYPE_LIST END


void make_variable(TYPE type, ID_LIST node) {
  TYPETAG tag;
  node = get_head(node);
  DTYPE_LIST dlist = dl_get_head(node->dlist);
  ST_ID id;
  TYPE type_holder = type;
  PARAM_LIST param_list;
  //msg("make_var called");

  // must have a type
  if(type) {
    tag = ty_query(type);
    while(node) {
      dlist = dl_get_head(node->dlist);
      while(dlist) {
        if(dlist->dtype == DECL_ARRAY) { // array in declaration list
          tag = ty_query(type);
          if(tag == TYFUNC) {
            error("cannot have an array of functions");
          } else {
            type = ty_build_array(type, TRUE, dlist->u.array.dim);
          }
        } else if(dlist->dtype == DECL_PTR) { // ptr in declaration list
          type = make_unary_expr(type, PTR);
        } else if(dlist->dtype == DECL_FUNC) {
          tag = ty_query(type);
          if(tag == TYFUNC) {
            error("cannot have function returning function");
          } else if(tag == TYARRAY) {
            error("cannot have function returning array");
          } else {
            param_list = param_get_head(dlist->u.func.param);
            type = ty_build_func(type, PROTOTYPE, param_list);
          }
        //  msg("func in tree");
        } else if(dlist->dtype == DECL_ID) { // id at end of declaration list
          id = dlist->u.identifier.id;
        //  msg("installed id: %s", st_get_id_str(id));
            install_dr(type, id);
        }
        dlist = dlist->next;
      }
      type = type_holder;
      node = node->next;
    } 
  }
}

void install_dr(TYPE type, ST_ID id) {
  ST_DR dr;
  BOOLEAN installed = FALSE;

  dr = stdr_alloc();
  dr->tag = GDECL;
  dr->u.decl.type = type;
  dr->u.decl.sc = NO_SC;
  dr->u.decl.err = FALSE;
  // binding and regno can be ignored

  installed = st_install(id, dr);
  if(installed) {
    //  msg("ID before encode: %s", st_get_id_str(id));
    if(ty_query(type) != TYFUNC) {
      encode(id); //probably need to move this for proj 2
    }
  } else {
    error("duplicate declaration for: %s", st_get_id_str(id));
  }
}

//TYPE start
// treat ptr and reference as unary operator
TYPE make_unary_expr(TYPE type, UNOP_TYPE op) {
  TYPETAG tag;

  tag = ty_query(type);
  switch(op) {
    case PTR:
      return ty_build_ptr(type, NO_QUAL);
    case REF:
      return ty_build_ptr(type, NO_QUAL);
    default:
      return;  
  }
  // case: is ptr op
}


TYPE build_type(DTYPE_LIST dlist, TYPE type) {
  TYPETAG tag;
  dlist = dl_get_head(dlist);
  while(dlist) {
    if(dlist->dtype == DECL_ARRAY) { // array in declaration list
      tag = ty_query(type);
      if(tag == TYFUNC) {
        error("cannot have an array of functions");
      } else {
        type = ty_build_array(type, TRUE, dlist->u.array.dim);
      }
    } else if(dlist->dtype == DECL_PTR) { // ptr in declaration list
      type = make_unary_expr(type, PTR);
    } else if(dlist->dtype == DECL_FUNC) {
      type = ty_build_func(type, PROTOTYPE, NULL);
    //  msg("func in tree");
    } 
    dlist = dlist->next;
  }
  return type;
}
//TYPE End

//PARAM_LIST Start
PARAM_LIST make_param_node(DTYPE_LIST dlist, TYPE type) {
  PARAM_LIST new_node = (PARAM_LIST)malloc(sizeof(PARAM));
  DTYPE_LIST dtail = dl_get_tail(dlist);
  BOOLEAN is_ref = FALSE;
  new_node->prev = NULL;
  new_node->next = NULL;
  if(dtail->u.identifier.id) {
    new_node->id = dtail->u.identifier.id;
  } else {
    error("no id in parameter list");
  }
  new_node->sc = NO_SC;
  new_node->err = FALSE;

  new_node->type = build_type(dlist, type);
  dlist = dl_get_head(dlist);
  while(dlist && !is_ref) {
    is_ref = dlist->dtype == DECL_REF;
    dlist = dlist->next;
  }
  new_node->is_ref = is_ref;
  
  return new_node;
}

PARAM_LIST append_param(PARAM_LIST list1, PARAM_LIST list2) {
  PARAM_LIST head;
  if (!list2) {
    return list1;
  }
  PARAM_LIST temp = param_get_head(list1);
  while(temp) {
    if(strcmp(st_get_id_str(temp->id), st_get_id_str(list2->id)) == 0) {
      error("duplicate parameter declaration for: %s", st_get_id_str(temp->id));
    }
    temp = temp->next; 
  }

  head = param_get_head(list2);
  list1 = param_get_tail(list1);
  if (list1) {
    head->prev = list1;
    list1->next = head;
  }

  return list2;
}

PARAM_LIST param_get_head(PARAM_LIST list) {
  PARAM_LIST head = list;

  while (head && head->prev) {
    head = head->prev;
  }
  return head;
}

PARAM_LIST param_get_tail(PARAM_LIST list) {
  PARAM_LIST tail = list;

  while (tail && tail->next) {
    tail = tail->next;
  }
  return tail;
}

long param_count(PARAM_LIST list) {
  PARAM_LIST next;
  long count;
  
  count = 0;
  
  next = param_get_head(list);
  while (next) {
    count++;
    next = next->next;
  }
  
  return count;
}
//PARAM_LIST End


//PROJECT TWO
//EXPR Start
EXPR error_expr() {
  EXPR new_expr = (EXPR)malloc(sizeof(TREENODE));
  
  new_expr->tag = ERROR_EXPR;
  new_expr->type = ty_build_basic(TYERROR);

  return new_expr;
}

EXPR make_int_expr(int val) {
  EXPR new_expr = (EXPR)malloc(sizeof(TREENODE));
 // msg("int expr");
  new_expr->tag = INT_EXPR;
  new_expr->type = ty_build_basic(TYUNSIGNEDINT);
  new_expr->u.val_int = val;


  return new_expr;
}

EXPR make_dbl_expr(double val) {
  EXPR new_expr = (EXPR)malloc(sizeof(TREENODE));
 // msg("double expr");

  new_expr->tag = DOUBLE_EXPR;
  new_expr->type = ty_build_basic(TYDOUBLE);
  new_expr->u.val_dbl = val;


  return new_expr;
}

EXPR make_id_expr(ST_ID id) {
  ST_DR dr;
  int block_num;
  EXPR new_expr = (EXPR)malloc(sizeof(TREENODE));
 // msg("identifier expr");

  dr = st_lookup(id, &block_num);
  if (!dr) {
    error("Undeclared identifier in expression: %s", st_get_id_str(id));
    return error_expr(); // otherwise crashes
  }

  new_expr->type = dr->u.decl.type;
 
  //if global variable declaration 
  if (dr->tag == GDECL) {
    new_expr->tag = GDECL_EXPR;
    new_expr->u.global_id = id;
  } else if (dr->tag == LDECL) { //inside a function (100%)
    //printf("declaring ldecl\n");
    new_expr->tag = LDECL_EXPR;
    new_expr->u.local_var.is_ref = dr->u.decl.is_ref;
   // new_expr->u.local_var.link_count = st_get_cur_block() - block_num;
  } else if (dr->tag == FDECL)  { //defining a function
    if (block_num <= 1) {
      new_expr->tag = GDECL_EXPR;
      new_expr->u.global_id = id;
    } else {
      new_expr->tag = FDECL_EXPR;
//      new_expr->u.local_func.link_count = st_get_cur_block() - block_num;
    }
  }

  return new_expr;
}

EXPR make_unop_expr(UNOP_TYPE op_type, EXPR arg) {
  ST_ID id;
  TYPE next;
  TYPETAG sub_expr_tag;
  EXPR new_expr;

 // msg("unop expr");

   if (arg->tag == ERROR_EXPR) {
    return error_expr();
  } 

  new_expr = (EXPR)malloc(sizeof(TREENODE));
  new_expr->tag = UNOP_EXPR;
  new_expr->type = arg->type;
  new_expr->u.unop.op = op_type;
  new_expr->u.unop.arg = arg;
  sub_expr_tag = ty_query(arg->type);
  //if a deref op, just return expr
  if (op_type = DEREF) {
    return new_expr; 
  }
  //EXPRESSION FOLDING HERE????
   if (op_type != CONVERT) {    
    //convert float to double
    if (sub_expr_tag == TYFLOAT) {
      msg("convert float -> double in unop");
      new_expr->u.unop.arg = make_convert_expr(arg, ty_build_basic(TYDOUBLE));
      sub_expr_tag = ty_query(new_expr->u.binop.left_arg->type);
      new_expr->type = ty_build_basic(TYDOUBLE);
    }
  }
  //error checking
  switch (op_type) { 
    case UPLUS: //type must be [int, float, double]
      if (sub_expr_tag != TYUNSIGNEDINT && sub_expr_tag != TYFLOAT && sub_expr_tag != TYDOUBLE) {
        error("Illegal type argument to unary plus");
        return error_expr();
      }
      break;
    case UMINUS: //type must be [int, float, double]
      if (sub_expr_tag != TYUNSIGNEDINT && sub_expr_tag != TYFLOAT && sub_expr_tag != TYDOUBLE) {
        error("Illegal type argument to unary minus");
        return error_expr();
      } 
      break;
  }

  return new_expr;
}

EXPR make_binop_expr(BINOP_TYPE op_type, EXPR left, EXPR right) {
  EXPR new_expr;
  TYPE next;

  if (left->tag == ERROR_EXPR || right->tag == ERROR_EXPR) {
    return error_expr();
  }

  new_expr = (EXPR)malloc(sizeof(TREENODE));
  new_expr->tag = BINOP_EXPR;
  new_expr->type = right->type;
  new_expr->u.binop.op = op_type;
  new_expr->u.binop.left_arg = left;
  new_expr->u.binop.right_arg = right;

  //EXPRESSION FOLDING HERE?

  //checks if a unary conversion is necessary
  TYPETAG left_tag = ty_query(left->type);
  TYPETAG right_tag = ty_query(right->type);

  
   if(left_tag == TYFLOAT) {
    new_expr->u.binop.left_arg = make_convert_expr(new_expr->u.binop.left_arg, ty_build_basic(TYDOUBLE));
    left_tag = ty_query(new_expr->u.binop.left_arg->type);
    new_expr->type = ty_build_basic(TYDOUBLE);
  } 
  if(right_tag == TYFLOAT) {
    new_expr->u.binop.right_arg = make_convert_expr(new_expr->u.binop.right_arg, ty_build_basic(TYDOUBLE));
    right_tag = ty_query(new_expr->u.binop.right_arg->type);
    new_expr->type = ty_build_basic(TYDOUBLE);
  } 

  if(left_tag == TYFLOAT || right_tag == TYFLOAT) {
   /* if(left_tag == TYSIGNEDLONGINT || left_tag == TYSIGNEDINT || left_tag == TYUNSIGNEDINT) {
      new_expr->u.binop.left_arg = make_convert_expr(new_expr->u.binop.left_arg, ty_build_basic(TYFLOAT));
      left_tag = ty_query(new_expr->u.binop.left_arg->type);
      new_expr->type = ty_build_basic(TYFLOAT);
    } else if(right_tag == TYSIGNEDLONGINT || right_tag == TYSIGNEDINT || right_tag == TYUNSIGNEDINT) {
      new_expr->u.binop.right_arg = make_convert_expr(new_expr->u.binop.right_arg, ty_build_basic(TYFLOAT));
      right_tag = ty_query(new_expr->u.binop.right_arg->type);
      new_expr->type = ty_build_basic(TYFLOAT);
    }*/
      msg("should not be called");
  } else if(left_tag == TYDOUBLE || right_tag == TYDOUBLE) {
    if(left_tag == TYSIGNEDLONGINT || left_tag == TYSIGNEDINT || left_tag == TYUNSIGNEDINT) {
      new_expr->u.binop.left_arg = make_convert_expr(new_expr->u.binop.left_arg, ty_build_basic(TYDOUBLE));
      left_tag = ty_query(new_expr->u.binop.left_arg->type);
      new_expr->type = ty_build_basic(TYDOUBLE);
    } else if(right_tag == TYSIGNEDLONGINT || right_tag == TYSIGNEDINT || right_tag == TYUNSIGNEDINT) {
      new_expr->u.binop.right_arg = make_convert_expr(new_expr->u.binop.right_arg, ty_build_basic(TYDOUBLE));
      right_tag = ty_query(new_expr->u.binop.right_arg->type);
      new_expr->type = ty_build_basic(TYDOUBLE);
    }
  }

  //check if lval is required (for assign ops)
  if (op_type == ASSIGN) {
    //msg("ASSIGN \n");
    
    //make sure left expr is an lval
    if (is_lval(left) != TRUE) {
      error("Assignment requires l-value on the left");
      return error_expr(); 
    }
    
    //make sure right expr is lval
    if (is_lval(right))  {
      //msg("DEREF THINGY \n");
      new_expr->u.binop.right_arg = make_unop_expr(DEREF, right); 
    }

     /*if (left_tag == TYFLOAT) {
        //need to convert so they're both doubles
        if(left_tag == TYFLOAT && left_tag != right_tag) {
          new_expr->u.binop.left_arg = make_convert_expr(new_expr->u.binop.left_arg, ty_build_basic(TYFLOAT));
        } 

     }*/
    if(left_tag == TYFLOAT) {
      new_expr->u.binop.right_arg = make_convert_expr(new_expr->u.binop.right_arg, ty_build_basic(TYFLOAT));
      new_expr->type = ty_build_basic(TYFLOAT);
    } 
    return new_expr;
  }

  //make sure both expr are rvals
  if (is_lval(left)) {
    new_expr->u.binop.left_arg = make_unop_expr(DEREF, left);
  }
  if (is_lval(right)) {
    new_expr->u.binop.right_arg = make_unop_expr(DEREF, right);
  }
  if(left_tag == TYUNSIGNEDCHAR || left_tag == TYSIGNEDCHAR) {
    new_expr->u.binop.left_arg = make_convert_expr(new_expr->u.binop.left_arg, ty_build_basic(TYSIGNEDINT));
  }
  //CONVERSIONS??
  //do special action for specific ops
  switch (op_type) {
    case PLUS: case MINUS: case TIMES: case DIV:
      //make sure both expr are numerical
      //at least one is a double?
    if(left_tag == TYFLOAT) {
      new_expr->u.binop.left_arg = make_convert_expr(new_expr->u.binop.left_arg, ty_build_basic(TYDOUBLE));
      left_tag = ty_query(new_expr->u.binop.left_arg->type);
      new_expr->type = ty_build_basic(TYDOUBLE);
    } 
    if(right_tag == TYFLOAT) {
      new_expr->u.binop.right_arg = make_convert_expr(new_expr->u.binop.right_arg, ty_build_basic(TYDOUBLE));
      right_tag = ty_query(new_expr->u.binop.right_arg->type);
      new_expr->type = ty_build_basic(TYDOUBLE);
    } 
      if (left_tag == TYDOUBLE || right_tag == TYDOUBLE) {
        //need to convert so they're both doubles
        if(left_tag == TYDOUBLE && left_tag != right_tag) {
          new_expr->u.binop.right_arg = make_convert_expr(new_expr->u.binop.right_arg, ty_build_basic(TYDOUBLE));
        } else if(right_tag == TYDOUBLE && left_tag != right_tag) {
          new_expr->u.binop.left_arg = make_convert_expr(new_expr->u.binop.left_arg, ty_build_basic(TYDOUBLE));
        }
        new_expr->type = ty_build_basic(TYDOUBLE);
      } else if (left_tag == TYFLOAT || right_tag == TYFLOAT) {
        /*//need to convert so they're both floats
        if(left_tag == TYFLOAT && left_tag != right_tag) {
          new_expr->u.binop.right_arg = make_convert_expr(new_expr->u.binop.right_arg, ty_build_basic(TYFLOAT));
        } else if(right_tag == TYFLOAT && left_tag != right_tag) {
          new_expr->u.binop.left_arg = make_convert_expr(new_expr->u.binop.left_arg, ty_build_basic(TYFLOAT));
        }
        new_expr->type = ty_build_basic(TYFLOAT);*/
        msg("should not be called");
      } else { //they're both ints
        new_expr->type = ty_build_basic(TYSIGNEDINT);
      }
      break;
    //comparison
    case EQUALS: case LESS_THAN: case LESS_EQUALS:
    case NOT_EQUALS: case GREATER_THAN: case GREATER_EQUALS:
      //make sure both expr have same type      
      //test if valid tagtype?? int, double, char...
      
      new_expr->type = ty_build_basic(TYSIGNEDINT);
      break;
  }

  return new_expr;
}

EXPR make_func_expr(EXPR func) {
  EXPR expr;
  TYPE return_type, next;
  PARAM_LIST params;
  PARAMSTYLE paramstyle;
  TYPETAG sub_expr_tag;
  //msg("make func expr");
  return_type = ty_query_func(func->type, &paramstyle, &params);
  
  expr = (EXPR)malloc(sizeof(TREENODE));
  expr->tag = FUNC_EXPR;
  expr->type = return_type;
 // expr->u.fcall.args = args;
  expr->u.fcall.function = func;

  return expr;
}


BOOLEAN is_lval(EXPR expr) {
  //get typetag from type
  TYPETAG tag = ty_query(expr->type);

  //if LVAR, it's lval
  if (expr->tag == LDECL_EXPR) {
    return TRUE;
  }
  
  //if tag is GCECL and typetag is a data type
  if (expr->tag == GDECL_EXPR && tag != TYFUNC && tag != TYERROR) { 
    return TRUE;
  }

  return FALSE;
}

void declare_func(DTYPE_LIST list, TYPE type) {
  DTYPE_LIST tail = dl_get_tail(list);
  char * func_name = st_get_id_str(tail->u.identifier.id);
  ST_DR dr;

  if(list->dtype == DECL_FUNC) {
    ST_ID id = dl_get_tail(list)->u.identifier.id;
    install_func(id, type);


    b_func_prologue(func_name);
    BOOLEAN installed = FALSE;
    st_enter_block();

    PARAM_LIST p_list = list->u.func.param;
    int offset = 0;;
    while(p_list) {
      offset = b_store_formal_param(ty_query(p_list->type));
      dr = stdr_alloc();
      dr->tag = PDECL;
      dr->u.decl.type = p_list->type;
      dr->u.decl.sc = NO_SC;
      dr->u.decl.err = FALSE;
      dr->u.decl.binding = offset;

      installed = st_install(p_list->id, dr);
      if(!installed) {
        error("duplicate parameter declaration for: %s", st_get_id_str(func_name));
      }
      p_list = p_list->next;
      offset = 0;
    }
  } else {
    error("not function type");
  }
}

void exit_func(DTYPE_LIST list, TYPE type) {
  TYPETAG return_tag;
  DTYPE_LIST tail = dl_get_tail(list);
  char * func_name = st_get_id_str(tail->u.identifier.id);
  
  return_tag = ty_query(type);

  b_encode_return(return_tag);
  //msg("return type");

  b_func_epilogue(func_name);
  st_exit_block();
}

void install_func(ST_ID id, TYPE type) {
  PARAM_LIST func_params;
  TYPE return_type;
  ST_DR dr; 
  int block;
  BOOLEAN installed = FALSE;

  type = ty_build_func(type, PROTOTYPE, NULL);
  ST_DR foundDR = st_lookup(id, &block);
  //should use FDECL
  if(!foundDR) {
    install_dr(type, id);
  } else {
    //change to FDECL
  }
  //make sure we installed function id
  /*installed = st_install(id, dr);
  if (!installed) {
    error("Duplicate forward or external function declaration");
  }*/
}

EXPR make_convert_expr(EXPR sub_expr, TYPE type) {
  EXPR expr = make_unop_expr(CONVERT, sub_expr);
  expr->type = type;

  return expr;
}

// Kelly - Switch statement project 3 for 100%
/*
//List of declarators: int x, y;
typedef struct case {
    int c;
    struct node *next, *prev;
} CASENODE, *CASE_NODE;
*/
CASE_NODE make_case_statement(int case_value, EXPR expr) {
  CASE_NODE new_node = (CASE_NODE)malloc(sizeof(CASENODE));

  new_node->c = case_value;
  new_node->e = expr;

  return new_node;
}

