/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_JACK_TAB_H_INCLUDED
# define YY_YY_JACK_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */


	#include "scanner.h"




/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TK_EOF = 0,
     TK_LP = 258,
     TK_RP = 259,
     TK_LB = 260,
     TK_RB = 261,
     TK_LC = 262,
     TK_RC = 263,
     TK_COMMA = 264,
     TK_SEMI = 265,
     TK_ASSIGN = 266,
     TK_DOT = 267,
     TK_PLUS = 268,
     TK_MINUS = 269,
     TK_MULTIPLY = 270,
     TK_DIVIDE = 271,
     TK_AND = 272,
     TK_OR = 273,
     TK_NOT = 274,
     TK_LT = 275,
     TK_GT = 276,
     TK_CLASS = 277,
     TK_CONSTRUCTOR = 278,
     TK_METHOD = 279,
     TK_FUNCTION = 280,
     TK_VAR = 281,
     TK_STATIC = 282,
     TK_FIELD = 283,
     TK_LET = 284,
     TK_DO = 285,
     TK_IF = 286,
     TK_ELSE = 287,
     TK_WHILE = 288,
     TK_RETURN = 289,
     TK_TRUE = 290,
     TK_FALSE = 291,
     TK_NULL = 292,
     TK_THIS = 293,
     TK_ID = 294,
     TK_NUM = 295
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{


	struct {
		char* sval;
		int   ival;
		int   type;
	}info;



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_JACK_TAB_H_INCLUDED  */
