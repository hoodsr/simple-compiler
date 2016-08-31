/*
 *
 * yacc/bison input for simplified C++ parser
 *
 */

%{

#include "defs.h"
#include "types.h"
#include "tree.h"
#include "symtab.h"
#include "bucket.h"
#include "message.h"
#include "backend-x86.h"

    int yylex();
    int yyerror(char *s);
%}

%union {
	BOOLEAN      			y_bool;
	int						y_int;
	double					y_double;
	char *					y_string;
	ST_ID 					y_stid;
	ID_LIST 				y_idlist;
	BUCKET_PTR  			y_bucket;				
	DTYPE_LIST 				y_dtypelist;
  TYPE_SPECIFIER 			y_typespec;
  PARAM_LIST 				y_paramlist;
  EXPR 					y_expr;
  BINOP_TYPE     y_binoptype;
};

%token IDENTIFIER SIZEOF INT_CONSTANT DOUBLE_CONSTANT STRING_LITERAL
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME
%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELIPSIS
%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token BAD

%start translation_unit

%type <y_typespec> type_specifier type_qualifier
%type <y_bucket> declaration_specifiers storage_class_specifier
%type <y_stid> identifier identifier_list
%type <y_idlist>  init_declarator_list
%type <y_dtypelist> direct_declarator init_declarator declarator 
%type <y_paramlist> parameter_list parameter_declaration parameter_type_list


//%type <y_int> constant_expr
%type <y_bool> pointer
%type <y_int> INT_CONSTANT
%type <y_double> DOUBLE_CONSTANT
%type <y_string> IDENTIFIER SIZEOF STRING_LITERAL if_action
%type <y_expr> expr primary_expr unary_expr conditional_expr logical_or_expr logical_and_expr constant_expr
%type <y_expr> inclusive_or_expr exclusive_or_expr and_expr equality_expr relational_expr shift_expr
%type <y_expr> cast_expr multiplicative_expr additive_expr postfix_expr assignment_expr expr_opt argument_expr_list_opt
%type <y_binoptype> assignment_operator

%%

//Because of type conflicts im going to change the IDENTIFIER and identifier
//  types to EXPR instead of ST_ID. Looking at hw2 this SEEMS ok. But dunno?


 //Project Two
 /*******************************
  * Expressions                 *
  *******************************/

/*
4-5-16 @ 5:00
Happening inside a function. Not a declaration. Maybe inside something like main. 
Look it up in the symbol table. If its not in the symbol table its a semantic error.
*/
primary_expr
	: identifier    									{int block; if(st_lookup($1, &block)) {$$ = make_id_expr($1); } else {error("identifier not in symbol table");}}
															      // Keep info symbol table gives back in node. For global variables need to keep stid. Keep type in node. (TYPE)  
	| INT_CONSTANT										{ $$ = make_int_expr($1); }  //The actual number itself
	| DOUBLE_CONSTANT									{ $$ = make_dbl_expr($1); }
	| STRING_LITERAL 									
	| '(' expr ')'  									{ $$ = $2;} 
	;

/*
4-5-16 @ 7:20
*/
postfix_expr
	: primary_expr										//returned from earlier thing, the variable/number
	| postfix_expr '[' expr ']'							//something like an Array
	| postfix_expr '(' argument_expr_list_opt ')'		{/*test sm tab*/$$ = make_func_expr($1);}
	| postfix_expr '.' identifier   					//something like tag.type
	| postfix_expr PTR_OP identifier 					//something like int * x
	| postfix_expr INC_OP  								//something like ++
	| postfix_expr DEC_OP   							//something like --
	;

argument_expr_list_opt
	: /* null derive */
	| argument_expr_list  								//something like a parameter
	;

argument_expr_list
	: assignment_expr  								
	| argument_expr_list ',' assignment_expr  	
	;

unary_expr
	: postfix_expr  		 							//something like ++ or --
	| INC_OP unary_expr									{ $$ = make_unop_expr(INC, $2); } //++ unary_expr
	| DEC_OP unary_expr      							{ $$ = make_unop_expr(DEC, $2); } //-- unary_expr
	| '&' cast_expr 									{ $$ = make_unop_expr(REF, $2); }
	| '*' cast_expr 									{ $$ = make_unop_expr(PTR, $2); }
	| '+' cast_expr 									{ $$ = make_unop_expr(UPLUS, $2); } 
	| '-' cast_expr 									{ $$ = make_unop_expr(UMINUS, $2); } //85%
	| '~' cast_expr 									{ $$ = $2; } //default for now
	| '!' cast_expr                   					{ $$ = $2; } //default for now
	| SIZEOF unary_expr									{ $$ = $2; }
	| SIZEOF '(' type_name ')'
	;

cast_expr
	: unary_expr
	| '(' type_name ')' cast_expr 						{ $$ = $4; } //DEFAULT
	;

multiplicative_expr
	: cast_expr 										//default
	| multiplicative_expr '*' cast_expr 				{ $$ = make_binop_expr(TIMES, $1, $3); } //85% multiplication using binop type
	| multiplicative_expr '/' cast_expr 				{ $$ = make_binop_expr(DIV, $1, $3); } //85% division using binop type
	| multiplicative_expr '%' cast_expr 				{ $$ = make_binop_expr(MOD, $1, $3); } //mod using binop type
	;

additive_expr
	: multiplicative_expr 								//default
	| additive_expr '+' multiplicative_expr				{ $$ = make_binop_expr(PLUS, $1, $3);}  //85% adding using binop type
	| additive_expr '-' multiplicative_expr 			{ $$ = make_binop_expr(MINUS, $1, $3); }  //85% subtracting using binop type
	;

shift_expr
	: additive_expr
	| shift_expr LEFT_OP additive_expr
	| shift_expr RIGHT_OP additive_expr
	;

relational_expr
	: shift_expr
	| relational_expr '<' shift_expr             { $$ = make_binop_expr(LESS_THAN, $1, $3);} 
	| relational_expr '>' shift_expr             { $$ = make_binop_expr(GREATER_THAN, $1, $3);} 
	| relational_expr LE_OP shift_expr           { $$ = make_binop_expr(LESS_EQUALS, $1, $3);} 
	| relational_expr GE_OP shift_expr           { $$ = make_binop_expr(GREATER_EQUALS, $1, $3);} 
	;

equality_expr
	: relational_expr
	| equality_expr EQ_OP relational_expr        { $$ = make_binop_expr(EQUALS, $1, $3);} 
	| equality_expr NE_OP relational_expr        { $$ = make_binop_expr(NOT_EQUALS, $1, $3);} 
	;

and_expr
	: equality_expr
	| and_expr '&' equality_expr
	;

exclusive_or_expr
	: and_expr
	| exclusive_or_expr '^' and_expr
	;

inclusive_or_expr
	: exclusive_or_expr
	| inclusive_or_expr '|' exclusive_or_expr
	;

logical_and_expr
	: inclusive_or_expr
	| logical_and_expr AND_OP inclusive_or_expr
	;

logical_or_expr
	: logical_and_expr
	| logical_or_expr OR_OP logical_and_expr
	;

conditional_expr
	: logical_or_expr
	| logical_or_expr '?' expr ':' conditional_expr
	;

assignment_expr
	: conditional_expr
	| unary_expr assignment_operator assignment_expr 			{ $$ = make_binop_expr($2, $1, $3);}
																/*NOOO long val = eval_expr($3); set_val($1, val); store_expr($3); reset_cache(); $$ = val;*/
	;

//Unary carries up to primary which carries up to an identifier. Assignment op is right below. Assignment_expr carries up to mul, add, etc.
//Above based on this snippet of code from a working hw2
/*
assign
    : VAR '=' expr		               {long val = eval_expr($3);
                                        set_val($1, val);
                                        store_expr($3);
                                        reset_cache();
                                        $$ = val;}

    | expr                             {store_expr($1);
                                        long val = eval_expr($1);
                                        update_cache($1);
                                        $$ = val;}
    ;
*/


assignment_operator
	: '='  						{$$ = ASSIGN;}	
	| MUL_ASSIGN  				{$$ = ASSIGN;}	
	| DIV_ASSIGN          		{$$ = ASSIGN;}	    
	| MOD_ASSIGN  				{$$ = ASSIGN;}		
	| ADD_ASSIGN  				{$$ = ASSIGN;}		
	| SUB_ASSIGN 				{$$ = ASSIGN;}		
	| LEFT_ASSIGN 				{$$ = ASSIGN;}		
	| RIGHT_ASSIGN  			{$$ = ASSIGN;}		
	| AND_ASSIGN  				{$$ = ASSIGN;}		
	| XOR_ASSIGN  				{$$ = ASSIGN;}		
	| OR_ASSIGN 				{$$ = ASSIGN;}		
	;


expr
	: assignment_expr 				//default?
	| expr ',' assignment_expr 		//maybe adding to a list?
	;

constant_expr
	: conditional_expr				//{$$ = make_binop_expr();}
	;

expr_opt
	: /* null derive */
	| expr
	;

 /*******************************
  * Declarations                *
  *******************************/

/*
build_base(bucket)
ty_build_basic(TYPETAG) - in this particular situation you wont use the second
  function. It's nice to have, and probably used in other places, but you wont
  be using it for the actual building of the type. Would use build_base (?) 
*/

declaration
	: declaration_specifiers ';' 									    {error("no declarator in declaration");} 
	| declaration_specifiers init_declarator_list ';' 					{if(!is_error_decl($1)) {TYPE type = build_base($1); make_variable(type, $2);}} 
	;                                                 

declaration_specifiers
	: storage_class_specifier 												//default as we dont deal with storage in proj 1
 	| storage_class_specifier declaration_specifiers   						//default as we dont deal with storage in proj 1
	| type_specifier  										        		{$$ = update_bucket(NULL, $1, NULL);}
	| type_specifier declaration_specifiers 								{$$ = update_bucket($2, $1, NULL);} 	 
	| type_qualifier                        					 			//default as we dont deal with storage in proj 1
	| type_qualifier declaration_specifiers									//default as we dont deal with storage in proj 1
	;

/*
Init_declarator_list: 
One or more declarators possibly initialized.
Example:
  Say "int x, *p;" Gives parse tree at 58 mins on 3-22-16
Now at 1:04:00. Talks about using $0 to access the declaration specifier that is used above. 
*/
init_declarator_list
	: init_declarator 														{$$ = make_id_node($1);}
	| init_declarator_list ',' init_declarator 								{$$ = append_id($3, $1);}
	;                                                       										 

/*
Lecture 3-24-16
right here talked about in the initiliazer section. 
  extra credit:
  Main point: put things in the right place on the semantic stack
  to avoid a ton of work. Put intermidiate action right between
  '=' and initializer. 

Also talks about this on 3-29-16. Basically says the same stuff. 
*/
init_declarator
	: declarator  //{$$=;} 																			//get the type of this declaration based on the type declarator and the bucket.
	| declarator '=' initializer //{$$=;} 															//$$=same as above. Stick this type of the semantic stack. right before the initlaizer
	;                            																	//HERE is where we use $0. Also use $<y_bucket>0. at 28 mins in lecture

storage_class_specifier
	: TYPEDEF 
	| EXTERN 
	| STATIC 
	| AUTO 
	| REGISTER 
	;

type_specifier
	: VOID 													{$$ = VOID_SPEC;}
	| CHAR 													{$$ = CHAR_SPEC;}
	| SHORT 												{$$ = SHORT_SPEC;}	
	| INT 													{$$ = INT_SPEC;}
	| LONG 													{$$ = LONG_SPEC;}
	| FLOAT 												{$$ = FLOAT_SPEC;}
	| DOUBLE 												{$$ = DOUBLE_SPEC;}
	| SIGNED 												{$$ = SIGNED_SPEC;}
	| UNSIGNED 												{$$ = UNSIGNED_SPEC;}
	| struct_or_union_specifier                     		{$$ = STRUCT_SPEC;}   
	| enum_specifier 										{$$ = ENUM_SPEC;}
	| TYPE_NAME 											{$$ = TYPENAME_SPEC;}
	;

struct_or_union_specifier
	: struct_or_union '{' struct_declaration_list '}'
	| struct_or_union identifier '{' struct_declaration_list '}'
	| struct_or_union identifier
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list_opt
	| type_qualifier specifier_qualifier_list_opt //dont deal with
	;

specifier_qualifier_list_opt
	: /* null derive */
	| specifier_qualifier_list
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expr
	| declarator ':' constant_expr
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM identifier '{' enumerator_list '}'
	| ENUM identifier
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: identifier
	| identifier '=' constant_expr
	;

type_qualifier
	: CONST 
	| VOLATILE 
	;

declarator
	: direct_declarator
	| pointer declarator 								{if($1) { $$ = prepend_dtype(DECL_REF, $2, NULL, 0, NULL);}
															else {$$ = prepend_dtype(DECL_PTR, $2, NULL, 0, NULL);}}
	;

direct_declarator
	: identifier 										{$$ = make_dtype_node(DECL_ID, $1, 0, NULL);} 
	| '(' declarator ')'								{$$ = $2;}
	| direct_declarator '[' ']'  						{error("illegal array dimension");}
	| direct_declarator '[' constant_expr ']'			{int num = $3->u.val_int; if(num >= 1) {$$ = prepend_dtype(DECL_ARRAY, $1, NULL, num, NULL);} 
														     else {error("illegal array dimension");}}
	| direct_declarator '(' parameter_type_list ')'  	{$$ = prepend_dtype(DECL_FUNC, $1, NULL, 0, $3);}                 
	| direct_declarator '(' ')' 						{$$ = prepend_dtype(DECL_FUNC, $1, NULL, 0, NULL);} 
	| direct_declarator '(' identifier_list ')'			{msg("this is called: direct_declarator '(' identifier_list ')'");}
	;

pointer
	: '*' specifier_qualifier_list_opt					{$$ = FALSE;}
 	| '&'												{$$ = TRUE;}
	;

//Defaults
parameter_type_list
	: parameter_list 											
	| parameter_list ',' ELIPSIS                               
	;

//TODO
//3-29-16 (35 mins)
parameter_list
	: parameter_declaration 							//PARAM_LIST type. Lists consisting of one node. Default. 
	| parameter_list ',' parameter_declaration          {$$ = append_param($1, $3);}
	;

//TODO
//3-29-15 (38 mins)
parameter_declaration
	: declaration_specifiers declarator                 {TYPE type = build_base($1); 
															BOOLEAN is_ref = FALSE;
															$$ = make_param_node($2, type);}
	| declaration_specifiers 							{$$=NULL; error("no id in parameter list");} //semantic errors. 
	| declaration_specifiers abstract_declarator 		{$$=NULL;} //semantic errors. 
	;

//TODO
//unsure if need anything besides defaults to pass it up the stack. 
identifier_list
	: identifier 														
	| identifier_list ',' identifier 				
	;

//unsure about whether necessary?
type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator  
	;

//unsure about whether necessary?
abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer abstract_declarator
	;

//unsure about whether necessary?
direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' constant_expr ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' constant_expr ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

//EXTRA CREDIT
//Lecture 3-24-16. 29 mins.
//type of object is at $<y_type>0
initializer 										
	: assignment_expr 								{;} //not default						
	| '{' initializer_list comma_opt '}'            {;} //not default
	;

    /*
      For assignment_expr 
      Always a numerical const. Int or floating point. Dont have to worry about operators right now.
      Check $0 to be appropriate base type (expected type $<y_type>0). 
      If no error then can start spitting out assembly code. b_alloc_gdata(); 39 mins 3-24-16
    */

	/*
	  For '{' initializer_list comma_opt '}' 
	  check between '{' and init_list that $0 is an array (ty_query($0)==TYARRAY). If not semantic error. If yes, 
	  then $$ = ty_query_array().(49 mins) Also says to do explicit typing again. AT THE END of this action. 
	  Push is matched by a pop in the same production.
	  about (1:02:00 mins) in. If count is still positive, means the initializer is short so you pad with allocations of 0.  
	    something about this being the only way we know were at the end of the list because we see the right brace.
	  One last thing, pop the global stack. 
	  He spent 10 mins flip flopping around on what to do here.
	  in ty_query_array() a parameter is dimflag - always be present. Put it somewhere in some variable you arnt going to use. 50 mins
      can make a dimflag variable called junk to pass in. Need it, but does nothing. 
	  other parameter is dim - sets the array size. 
	  to find the size of the array you may have to create a global stack of array sizes
      of unsigned ints. (56 mins). 
	  Also talks about thi on 3-29-16 at about (1:04:30)
	*/

//Default. 
comma_opt
	: /* Null derive */
	| ','
	;

//EXTRA CREDIT
//Lecture 3-24-16. 32 mins.
//Grammatically legal: {3, {4.2, 8}, 9, {{{16}}}} BUT, not symmantically correct because it it not uniform
//Have to prevent this. Use the symmantic stack to help. 
	//other symantic error int a[3] = {2,3,4,5}; because its too big. 
initializer_list
	: initializer 											{;} 	//decrement top of global stack. If negative, error. 
	| initializer_list ',' initializer  					{;}  	//decrement top of global stack. If negative, error. If 0, nothing more to do. 
	;

//FOR EXTRA CREDIT WATCH 3-24-16 and 3-29-16. He talks about how to do it in both. I stopped listening about 1:07:00 in
//3-29-16 though since ther was only 10 mins left on that lecture and it was extra credit. At this point he is going over
//how he did it himself. 

 //Doesnt appear to be used in project one. 
 /*******************************
  * Statements                  *
  *******************************/

//Switch statements
statement
	: labeled_statement
	| {st_enter_block();} compound_statement {st_exit_block();} //blocks inside functions declarations
	| expression_statement
	| selection_statement //if else and switch 
	| iteration_statement
	| jump_statement
	;

//Switch statements
labeled_statement
	: identifier ':' statement
	| CASE constant_expr ':' statement 				//{$$ = make_case_statement($2, $3);} //$2 is a int and/or string. And $3 is the action taken (should be expression_statement)
	| DEFAULT ':' statement
	;

//Used in project 2
compound_statement
	: '{' '}'																		//Default case, do nothing
	| '{' statement_list '}' 										//list of exprs
	| '{' declaration_list '}'									//list of declarations
	| '{' declaration_list statement_list '}'		//both decls and exprs
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

//Not used in proj1
statement_list
	: statement
	| statement_list statement
	;

expression_statement
	: expr_opt ';'											{encode_expr($1); }//$$=$1}
	/*traverse expr tree and output code. value gets pushed on stack b_pop()*/
	;

//proj 3
selection_statement
	: IF '(' expr ')' if_action statement                   {b_label($5);}
	| IF '(' expr ')' if_action statement ELSE              {char * end_label = new_symbol();
      													        b_jump(end_label);
      													        b_label($5);
                                                          		$<y_string>$ = end_label;}
	  statement                                             {b_label($<y_string>8);} // end if else
	| SWITCH '(' expr ')' statement                         //{$$ = encode_switch($2, $3);}
	;

if_action
  :  /*empty*/     											{char * exit_label = new_symbol();
	                   											encode_expr($<y_expr>-1);
	                   											b_cond_jump(TYUNSIGNEDINT, B_ZERO, exit_label); // unsure about TYUNSIGNEDINT as expr type
	                   											$$ = exit_label;} 
  ;

//proj 3
iteration_statement
	: WHILE '(' expr ')'                            		{char * end_label = new_symbol();
		                                               			add_label(end_label);
                                                   				$<y_string>$ = end_label;}  // two different intermediate actions
                                                  			{char * start_label = new_symbol();
                                                  				b_label(start_label);
                                                   				encode_expr($3);
                                                   				b_cond_jump(TYSIGNEDINT, B_ZERO, $<y_string>5);
                                                   				$<y_string>$ = start_label;}  // b/c two labels: start and end
	statement                                       		{b_jump($<y_string>6);
		                                              			b_label($<y_string>5);}
	| DO statement WHILE '(' expr ')' ';'
	| FOR '(' expr_opt ';' expr_opt ';' expr_opt ')' 		{char *exit_label = new_symbol();
		                                                		add_label(exit_label);
		                                                		if($3->tag) {
		                                                			encode_expr($3);
		                                                  			b_pop();
		                                                		}// else msg("else reached yay");
                                                    			$<y_string>$ = exit_label;} //action 1 for exit label
                                                   			{char * start_label = new_symbol();
                                                    			b_label(start_label);
                                                    			if($3->tag) {
                                                    				encode_expr($5);
                                                    			} else {
                                                    				EXPR one = make_int_expr(1);
                                                    				encode_expr(one);
                                                    			}
                                                    			b_cond_jump(TYSIGNEDINT, B_ZERO, $<y_string>9);
                                                    			$<y_string>$ = start_label;} 
    statement                                      				{if($3->tag) {
    	                                               				encode_expr($7);
    	                                                			b_pop();
    	                                              			}
    	                                             			b_jump($<y_string>10);
		                                               			b_label($<y_string>9);}
	;

//proj 3
jump_statement
	: GOTO identifier ';'
	| CONTINUE ';'
	| BREAK ';'                        						{char * label = get_exit_label();
	                                    						b_jump(label);} //jump to exit label} //must occur inside while, switch, or for loop
	| RETURN expr_opt ';'              						{encode_expr($2); 
                                      						} //jump to end of func
	;

 //Doesnt appear to be used in project one. 
 /*******************************
  * Top level                   *
  *******************************/

//Not used in proj1
translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

//Not used in proj1
external_declaration
	: function_definition
	| declaration
	;

//Used in proj 2
function_definition
	: declarator {declare_func($1, ty_build_basic(TYSIGNEDINT));} compound_statement {exit_func($1, ty_build_basic(TYSIGNEDINT));}             
	| declaration_specifiers declarator  {declare_func($2, build_base($1));} compound_statement {exit_func($2, build_base($1));} 
	;

 //Used in proj1. But really necessary for each project. 
 /*******************************
  * Identifiers                 *
  *******************************/

identifier
	: IDENTIFIER                                     {$$ = st_enter_id($1);}
	;
%%

extern int column;

int yyerror(char *s)
{
	error("%s (column %d)",s,column);
        return 0;  /* never reached */
}