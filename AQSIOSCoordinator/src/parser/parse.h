
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     RW_REGISTER = 258,
     RW_STREAM = 259,
     RW_RELATION = 260,
     RW_ISTREAM = 261,
     RW_DSTREAM = 262,
     RW_RSTREAM = 263,
     RW_SELECT = 264,
     RW_DISTINCT = 265,
     RW_FROM = 266,
     RW_WHERE = 267,
     RW_GROUP = 268,
     RW_BY = 269,
     RW_AND = 270,
     RW_AS = 271,
     RW_UNION = 272,
     RW_EXCEPT = 273,
     RW_AVG = 274,
     RW_MIN = 275,
     RW_MAX = 276,
     RW_COUNT = 277,
     RW_SUM = 278,
     RW_ROWS = 279,
     RW_RANGE = 280,
     RW_NOW = 281,
     RW_PARTITION = 282,
     RW_UNBOUNDED = 283,
     RW_SECOND = 284,
     RW_MINUTE = 285,
     RW_HOUR = 286,
     RW_DAY = 287,
     T_EQ = 288,
     T_LT = 289,
     T_LE = 290,
     T_GT = 291,
     T_GE = 292,
     T_NE = 293,
     RW_INTEGER = 294,
     RW_FLOAT = 295,
     RW_CHAR = 296,
     RW_BYTE = 297,
     NOTOKEN = 298,
     T_INT = 299,
     T_REAL = 300,
     T_STRING = 301,
     T_QSTRING = 302
   };
#endif
/* Tokens.  */
#define RW_REGISTER 258
#define RW_STREAM 259
#define RW_RELATION 260
#define RW_ISTREAM 261
#define RW_DSTREAM 262
#define RW_RSTREAM 263
#define RW_SELECT 264
#define RW_DISTINCT 265
#define RW_FROM 266
#define RW_WHERE 267
#define RW_GROUP 268
#define RW_BY 269
#define RW_AND 270
#define RW_AS 271
#define RW_UNION 272
#define RW_EXCEPT 273
#define RW_AVG 274
#define RW_MIN 275
#define RW_MAX 276
#define RW_COUNT 277
#define RW_SUM 278
#define RW_ROWS 279
#define RW_RANGE 280
#define RW_NOW 281
#define RW_PARTITION 282
#define RW_UNBOUNDED 283
#define RW_SECOND 284
#define RW_MINUTE 285
#define RW_HOUR 286
#define RW_DAY 287
#define T_EQ 288
#define T_LT 289
#define T_LE 290
#define T_GT 291
#define T_GE 292
#define T_NE 293
#define RW_INTEGER 294
#define RW_FLOAT 295
#define RW_CHAR 296
#define RW_BYTE 297
#define NOTOKEN 298
#define T_INT 299
#define T_REAL 300
#define T_STRING 301
#define T_QSTRING 302




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 23 "parse.yy"

  int   ival;
  float rval;
  char *sval;
  NODE *node;



/* Line 1676 of yacc.c  */
#line 155 "parse.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


