/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_SRC_GRAMMAR_H_INCLUDED
# define YY_YY_SRC_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 6 "src/grammar.y" /* yacc.c:1921  */

  #define YYSTYPE ast_t
  typedef struct yypstate yypstate;
  typedef struct pctx pctx_t;
  typedef struct pctx
  {
      yypstate *ps;
      bool done;
      ast_t result;
      bool expr;
      unsigned nest;
      bool brk;
      bool ifexpr;
      int ltok;
      ast_t lval;
      char *buf;
      int (*get)(pctx_t *);
      void (*unget)(int, pctx_t *);
      void *data;
  } pctx_t;
  extern ast_t ast_null;
  extern const int tok_empty;
  #define L_BUFSIZE 256
  int yylex(YYSTYPE *out, pctx_t *ctx);
  void yyerror(pctx_t *ctx, const char *err);

#line 75 "src/grammar.h" /* yacc.c:1921  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    LIT_INT = 258,
    LIT_DBL = 259,
    LIT_BOOL = 260,
    LIT_SYM = 261,
    LIT_STR = 262,
    ID = 263,
    TYPE = 264,
    FUNCTION = 265,
    WHILE = 266,
    FOR = 267,
    BREAK = 268,
    CONTINUE = 269,
    LET = 270,
    VAR = 271,
    GLOBAL = 272,
    CONST = 273,
    INCLUDE = 274,
    LEXERR = 275,
    RET = 276,
    IF = 277,
    ELSE = 278,
    OR = 279,
    AND = 280,
    GE = 281,
    LE = 282,
    EQ = 283,
    NE = 284,
    UMINUS = 285,
    UPLUS = 286
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



#ifndef YYPUSH_MORE_DEFINED
# define YYPUSH_MORE_DEFINED
enum { YYPUSH_MORE = 4 };
#endif

typedef struct yypstate yypstate;

int yyparse (pctx_t *ctx);
int yypush_parse (yypstate *ps, int pushed_char, YYSTYPE const *pushed_val, pctx_t *ctx);
int yypull_parse (yypstate *ps, pctx_t *ctx);
yypstate * yypstate_new (void);
void yypstate_delete (yypstate *ps);

#endif /* !YY_YY_SRC_GRAMMAR_H_INCLUDED  */
