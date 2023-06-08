%{

	#include "jack.tab.h"

	#include <stdio.h>

	#define YYERROR_VERBOSE

	#define _CONST_   1
	#define _INT_     2
	#define _BOOLEAN_ 3
	#define _CHAR_    4
	#define _VOID_    5

	#define SET_CONST(T)   T.type = _CONST_
	#define SET_INT(T)     T.type = _INT_
	#define SET_BOOLEAN(T) T.type = _BOOLEAN_
	#define SET_CHAR(T)    T.type = _CHAR_
	#define SET_VOID(T)    T.type = _VOID_

	#define IS_CONST(T)   T.type == _CONST_
	#define IS_INT(T)     T.type == _INT_
	#define IS_BOOLEAN(T) T.type == _BOOLEAN_
	#define IS_CHAR(T)    T.type == _CHAR_
	#define IS_VOID(T)    T.type == _VOID_

	#define TYPE(T1, T2) ( T1 | ( T2 << 8 ) )

	int line = 0;

	int yyerror(const char* msg){
		printf("%s on line %d\n", msg, line);
		return 0;
	}

%}

%code requires{
	#include "scanner.h"
}

%union{
	struct {
		char* sval;
		int   ival;
		int   type;
	}info;
}

%token TK_EOF 0

%token TK_LP TK_RP
%token TK_LB TK_RB
%token TK_LC TK_RC
%token TK_COMMA
%token TK_SEMI
%token TK_ASSIGN
%token TK_DOT

%token TK_PLUS
%token TK_MINUS
%token TK_MULTIPLY
%token TK_DIVIDE
%token TK_AND
%token TK_OR
%token TK_NOT
%token TK_LT
%token TK_GT

%token TK_CLASS TK_CONSTRUCTOR TK_METHOD TK_FUNCTION
%token TK_VAR TK_STATIC TK_FIELD
%token TK_LET TK_DO TK_IF TK_ELSE TK_WHILE TK_RETURN
%token TK_TRUE TK_FALSE TK_NULL
%token TK_THIS

%token <info> TK_ID
%token <info> TK_NUM

%%

start: {}
	| class
	;

class:
	TK_CLASS TK_ID TK_LC class_block TK_RC
	;

class_block:
	  var_dec
	| functions
	;

var_dec:
	  scope[S] TK_ID[T] TK_ID[ID] TK_SEMI
	| var_dec scope[S] TK_ID[T] TK_ID[ID] TK_SEMI
	| var_dec TK_COMMA TK_ID[ID]
	;

scope:
	  TK_FIELD
	| TK_STATIC
	| TK_VAR
	;

functions:
	  func_type[F] TK_ID[T] TK_ID[N] TK_LP arg_dec TK_RP TK_LC block TK_RC
	| functions func_type[F] TK_ID[T] TK_ID[N] TK_LP arg_dec TK_RP TK_LC block TK_RC
	;

func_type:
	  TK_METHOD
	| TK_FUNCTION
	| TK_CONSTRUCTOR
	;

arg_dec: {}
	| TK_ID[T] TK_ID[N]
	| arg_list TK_COMMA TK_ID[T] TK_ID[N]
	;

arg_list: {}
	| TK_ID[N]
	| arg_list TK_COMMA TK_ID[N]
	;

block: {}
	| TK_DO fn_call TK_SEMI
	| let_stmt
	| if_stmt
	| while_stmt
	| var_dec
	;
	
let_stmt:
	  TK_LET TK_ID TK_ASSIGN bin_expr TK_SEMI
	| TK_LET TK_ID TK_LP bin_expr TK_RP TK_ASSIGN bin_expr TK_SEMI
	;

fn_call:
	TK_ID[C] TK_DOT TK_ID[N] TK_LP arg_list TK_RP
	;

if_stmt:
	TK_IF TK_LP bin_expr TK_RP TK_LC block TK_RC else
	;
	
else:
	| TK_ELSE TK_LC block TK_RC
	;

while_stmt:
	TK_WHILE TK_LP bin_expr TK_RP TK_LC block TK_RC
	;

bin_expr:
	  bin_not
	| bin_not TK_AND bin_expr
	| bin_not TK_OR  bin_expr
	;

bin_not:
	  rel_expr
	| TK_NOT rel_expr
	;

rel_expr:
	  expr
	| expr TK_LT rel_expr
	| expr TK_GT rel_expr
	| expr TK_ASSIGN rel_expr
	;

expr:
	  term
	| term TK_PLUS expr
	| term TK_MINUS expr
	;

term:
	  factor
	| factor TK_MULTIPLY term
	| factor TK_DIVIDE term
	;

factor:
	  TK_NUM
	| TK_ID
	| TK_ID TK_LB bin_expr TK_RB
	| TK_ID TK_DOT TK_ID TK_LP arg_list TK_RP
	| TK_TRUE
	| TK_FALSE
	| TK_NULL
	;