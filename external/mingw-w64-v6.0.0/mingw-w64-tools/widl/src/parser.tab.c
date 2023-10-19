/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         parser_parse
#define yylex           parser_lex
#define yyerror         parser_error
#define yydebug         parser_debug
#define yynerrs         parser_nerrs

#define yylval          parser_lval
#define yychar          parser_char

/* Copy the first part of user declarations.  */
#line 1 "parser.y" /* yacc.c:339  */

/*
 * IDL Compiler
 *
 * Copyright 2002 Ove Kaaven
 * Copyright 2006-2008 Robert Shearman
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "widl.h"
#include "utils.h"
#include "parser.h"
#include "header.h"
#include "typelib.h"
#include "typegen.h"
#include "expr.h"
#include "typetree.h"

static unsigned char pointer_default = FC_UP;

typedef struct list typelist_t;
struct typenode {
  type_t *type;
  struct list entry;
};

struct _import_t
{
  char *name;
  int import_performed;
};

typedef struct _decl_spec_t
{
  type_t *type;
  attr_list_t *attrs;
  enum storage_class stgclass;
} decl_spec_t;

typelist_t incomplete_types = LIST_INIT(incomplete_types);

static void fix_incomplete(void);
static void fix_incomplete_types(type_t *complete_type);

static str_list_t *append_str(str_list_t *list, char *str);
static attr_list_t *append_attr(attr_list_t *list, attr_t *attr);
static attr_list_t *append_attr_list(attr_list_t *new_list, attr_list_t *old_list);
static decl_spec_t *make_decl_spec(type_t *type, decl_spec_t *left, decl_spec_t *right, attr_t *attr, enum storage_class stgclass);
static attr_t *make_attr(enum attr_type type);
static attr_t *make_attrv(enum attr_type type, unsigned int val);
static attr_t *make_attrp(enum attr_type type, void *val);
static expr_list_t *append_expr(expr_list_t *list, expr_t *expr);
static array_dims_t *append_array(array_dims_t *list, expr_t *expr);
static var_t *declare_var(attr_list_t *attrs, decl_spec_t *decl_spec, const declarator_t *decl, int top);
static var_list_t *set_var_types(attr_list_t *attrs, decl_spec_t *decl_spec, declarator_list_t *decls);
static ifref_list_t *append_ifref(ifref_list_t *list, ifref_t *iface);
static ifref_t *make_ifref(type_t *iface);
static var_list_t *append_var_list(var_list_t *list, var_list_t *vars);
static declarator_list_t *append_declarator(declarator_list_t *list, declarator_t *p);
static declarator_t *make_declarator(var_t *var);
static type_t *make_safearray(type_t *type);
static typelib_t *make_library(const char *name, const attr_list_t *attrs);
static type_t *append_ptrchain_type(type_t *ptrchain, type_t *type);
static warning_list_t *append_warning(warning_list_t *, int);

static type_t *reg_typedefs(decl_spec_t *decl_spec, var_list_t *names, attr_list_t *attrs);
static type_t *find_type_or_error(const char *name, int t);
static type_t *find_type_or_error2(char *name, int t);

static var_t *reg_const(var_t *var);

static void push_namespace(const char *name);
static void pop_namespace(const char *name);

static char *gen_name(void);
static void check_arg_attrs(const var_t *arg);
static void check_statements(const statement_list_t *stmts, int is_inside_library);
static void check_all_user_types(const statement_list_t *stmts);
static attr_list_t *check_iface_attrs(const char *name, attr_list_t *attrs);
static attr_list_t *check_function_attrs(const char *name, attr_list_t *attrs);
static attr_list_t *check_typedef_attrs(attr_list_t *attrs);
static attr_list_t *check_enum_attrs(attr_list_t *attrs);
static attr_list_t *check_struct_attrs(attr_list_t *attrs);
static attr_list_t *check_union_attrs(attr_list_t *attrs);
static attr_list_t *check_field_attrs(const char *name, attr_list_t *attrs);
static attr_list_t *check_library_attrs(const char *name, attr_list_t *attrs);
static attr_list_t *check_dispiface_attrs(const char *name, attr_list_t *attrs);
static attr_list_t *check_module_attrs(const char *name, attr_list_t *attrs);
static attr_list_t *check_coclass_attrs(const char *name, attr_list_t *attrs);
const char *get_attr_display_name(enum attr_type type);
static void add_explicit_handle_if_necessary(const type_t *iface, var_t *func);
static void check_def(const type_t *t);

static statement_t *make_statement(enum statement_type type);
static statement_t *make_statement_type_decl(type_t *type);
static statement_t *make_statement_reference(type_t *type);
static statement_t *make_statement_declaration(var_t *var);
static statement_t *make_statement_library(typelib_t *typelib);
static statement_t *make_statement_pragma(const char *str);
static statement_t *make_statement_cppquote(const char *str);
static statement_t *make_statement_importlib(const char *str);
static statement_t *make_statement_module(type_t *type);
static statement_t *make_statement_typedef(var_list_t *names);
static statement_t *make_statement_import(const char *str);
static statement_list_t *append_statement(statement_list_t *list, statement_t *stmt);
static statement_list_t *append_statements(statement_list_t *, statement_list_t *);
static attr_list_t *append_attribs(attr_list_t *, attr_list_t *);

static struct namespace global_namespace = {
    NULL, NULL, LIST_INIT(global_namespace.entry), LIST_INIT(global_namespace.children)
};

static struct namespace *current_namespace = &global_namespace;


#line 212 "parser.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int parser_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    aIDENTIFIER = 258,
    aPRAGMA = 259,
    aKNOWNTYPE = 260,
    aNUM = 261,
    aHEXNUM = 262,
    aDOUBLE = 263,
    aSTRING = 264,
    aWSTRING = 265,
    aSQSTRING = 266,
    aUUID = 267,
    aEOF = 268,
    SHL = 269,
    SHR = 270,
    MEMBERPTR = 271,
    EQUALITY = 272,
    INEQUALITY = 273,
    GREATEREQUAL = 274,
    LESSEQUAL = 275,
    LOGICALOR = 276,
    LOGICALAND = 277,
    ELLIPSIS = 278,
    tAGGREGATABLE = 279,
    tALLOCATE = 280,
    tANNOTATION = 281,
    tAPPOBJECT = 282,
    tASYNC = 283,
    tASYNCUUID = 284,
    tAUTOHANDLE = 285,
    tBINDABLE = 286,
    tBOOLEAN = 287,
    tBROADCAST = 288,
    tBYTE = 289,
    tBYTECOUNT = 290,
    tCALLAS = 291,
    tCALLBACK = 292,
    tCASE = 293,
    tCDECL = 294,
    tCHAR = 295,
    tCOCLASS = 296,
    tCODE = 297,
    tCOMMSTATUS = 298,
    tCONST = 299,
    tCONTEXTHANDLE = 300,
    tCONTEXTHANDLENOSERIALIZE = 301,
    tCONTEXTHANDLESERIALIZE = 302,
    tCONTROL = 303,
    tCPPQUOTE = 304,
    tDECODE = 305,
    tDEFAULT = 306,
    tDEFAULTBIND = 307,
    tDEFAULTCOLLELEM = 308,
    tDEFAULTVALUE = 309,
    tDEFAULTVTABLE = 310,
    tDISABLECONSISTENCYCHECK = 311,
    tDISPLAYBIND = 312,
    tDISPINTERFACE = 313,
    tDLLNAME = 314,
    tDOUBLE = 315,
    tDUAL = 316,
    tENABLEALLOCATE = 317,
    tENCODE = 318,
    tENDPOINT = 319,
    tENTRY = 320,
    tENUM = 321,
    tERRORSTATUST = 322,
    tEXPLICITHANDLE = 323,
    tEXTERN = 324,
    tFALSE = 325,
    tFASTCALL = 326,
    tFAULTSTATUS = 327,
    tFLOAT = 328,
    tFORCEALLOCATE = 329,
    tHANDLE = 330,
    tHANDLET = 331,
    tHELPCONTEXT = 332,
    tHELPFILE = 333,
    tHELPSTRING = 334,
    tHELPSTRINGCONTEXT = 335,
    tHELPSTRINGDLL = 336,
    tHIDDEN = 337,
    tHYPER = 338,
    tID = 339,
    tIDEMPOTENT = 340,
    tIGNORE = 341,
    tIIDIS = 342,
    tIMMEDIATEBIND = 343,
    tIMPLICITHANDLE = 344,
    tIMPORT = 345,
    tIMPORTLIB = 346,
    tIN = 347,
    tIN_LINE = 348,
    tINLINE = 349,
    tINPUTSYNC = 350,
    tINT = 351,
    tINT3264 = 352,
    tINT64 = 353,
    tINTERFACE = 354,
    tLCID = 355,
    tLENGTHIS = 356,
    tLIBRARY = 357,
    tLICENSED = 358,
    tLOCAL = 359,
    tLONG = 360,
    tMAYBE = 361,
    tMESSAGE = 362,
    tMETHODS = 363,
    tMODULE = 364,
    tNAMESPACE = 365,
    tNOCODE = 366,
    tNONBROWSABLE = 367,
    tNONCREATABLE = 368,
    tNONEXTENSIBLE = 369,
    tNOTIFY = 370,
    tNOTIFYFLAG = 371,
    tNULL = 372,
    tOBJECT = 373,
    tODL = 374,
    tOLEAUTOMATION = 375,
    tOPTIMIZE = 376,
    tOPTIONAL = 377,
    tOUT = 378,
    tPARTIALIGNORE = 379,
    tPASCAL = 380,
    tPOINTERDEFAULT = 381,
    tPRAGMA_WARNING = 382,
    tPROGID = 383,
    tPROPERTIES = 384,
    tPROPGET = 385,
    tPROPPUT = 386,
    tPROPPUTREF = 387,
    tPROXY = 388,
    tPTR = 389,
    tPUBLIC = 390,
    tRANGE = 391,
    tREADONLY = 392,
    tREF = 393,
    tREGISTER = 394,
    tREPRESENTAS = 395,
    tREQUESTEDIT = 396,
    tRESTRICTED = 397,
    tRETVAL = 398,
    tSAFEARRAY = 399,
    tSHORT = 400,
    tSIGNED = 401,
    tSIZEIS = 402,
    tSIZEOF = 403,
    tSMALL = 404,
    tSOURCE = 405,
    tSTATIC = 406,
    tSTDCALL = 407,
    tSTRICTCONTEXTHANDLE = 408,
    tSTRING = 409,
    tSTRUCT = 410,
    tSWITCH = 411,
    tSWITCHIS = 412,
    tSWITCHTYPE = 413,
    tTHREADING = 414,
    tTRANSMITAS = 415,
    tTRUE = 416,
    tTYPEDEF = 417,
    tUIDEFAULT = 418,
    tUNION = 419,
    tUNIQUE = 420,
    tUNSIGNED = 421,
    tUSESGETLASTERROR = 422,
    tUSERMARSHAL = 423,
    tUUID = 424,
    tV1ENUM = 425,
    tVARARG = 426,
    tVERSION = 427,
    tVIPROGID = 428,
    tVOID = 429,
    tWCHAR = 430,
    tWIREMARSHAL = 431,
    tAPARTMENT = 432,
    tNEUTRAL = 433,
    tSINGLE = 434,
    tFREE = 435,
    tBOTH = 436,
    CAST = 437,
    PPTR = 438,
    POS = 439,
    NEG = 440,
    ADDRESSOF = 441
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 138 "parser.y" /* yacc.c:355  */

	attr_t *attr;
	attr_list_t *attr_list;
	str_list_t *str_list;
	expr_t *expr;
	expr_list_t *expr_list;
	array_dims_t *array_dims;
	type_t *type;
	var_t *var;
	var_list_t *var_list;
	declarator_t *declarator;
	declarator_list_t *declarator_list;
	statement_t *statement;
	statement_list_t *stmt_list;
	warning_t *warning;
	warning_list_t *warning_list;
	ifref_t *ifref;
	ifref_list_t *ifref_list;
	char *str;
	UUID *uuid;
	unsigned int num;
	double dbl;
	interface_info_t ifinfo;
	typelib_t *typelib;
	struct _import_t *import;
	struct _decl_spec_t *declspec;
	enum storage_class stgclass;

#line 465 "parser.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE parser_lval;

int parser_parse (void);



/* Copy the second part of user declarations.  */

#line 482 "parser.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2995

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  211
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  105
/* YYNRULES -- Number of rules.  */
#define YYNRULES  394
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  691

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   441

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   195,     2,     2,     2,   194,   187,     2,
     208,   209,   192,   191,   182,   190,   202,   193,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   184,   207,
     188,   210,   189,   183,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   203,     2,   204,   186,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   205,   185,   206,   196,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   197,   198,   199,
     200,   201
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   321,   321,   335,   336,   336,   338,   339,   340,   343,
     346,   347,   348,   351,   352,   353,   353,   355,   356,   357,
     360,   361,   362,   363,   366,   367,   370,   371,   375,   376,
     377,   378,   379,   380,   381,   384,   395,   396,   400,   401,
     402,   403,   404,   405,   406,   407,   408,   411,   413,   421,
     427,   431,   432,   434,   438,   445,   446,   449,   450,   453,
     454,   458,   463,   470,   474,   475,   478,   479,   483,   486,
     487,   488,   491,   492,   495,   496,   497,   498,   499,   500,
     501,   502,   503,   504,   505,   506,   507,   508,   509,   510,
     511,   512,   513,   514,   515,   516,   517,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   529,   530,
     531,   532,   533,   534,   535,   536,   537,   538,   539,   540,
     541,   542,   543,   544,   545,   546,   547,   548,   549,   550,
     551,   552,   553,   554,   555,   556,   557,   558,   559,   560,
     561,   562,   563,   564,   565,   566,   567,   568,   569,   570,
     574,   575,   576,   577,   578,   579,   580,   581,   582,   583,
     584,   585,   586,   587,   588,   589,   590,   591,   592,   593,
     594,   595,   596,   597,   601,   602,   607,   608,   609,   610,
     613,   614,   617,   621,   627,   628,   629,   632,   636,   648,
     652,   657,   660,   661,   664,   665,   668,   669,   670,   671,
     672,   673,   674,   675,   676,   677,   678,   679,   680,   681,
     682,   683,   684,   685,   686,   687,   688,   689,   690,   691,
     692,   693,   694,   695,   696,   697,   698,   699,   700,   701,
     702,   703,   704,   705,   707,   709,   710,   713,   714,   717,
     723,   729,   730,   733,   738,   745,   746,   749,   750,   754,
     755,   758,   762,   768,   776,   780,   785,   786,   789,   790,
     791,   794,   796,   799,   800,   801,   802,   803,   804,   805,
     806,   807,   808,   809,   812,   813,   816,   817,   818,   819,
     820,   821,   822,   823,   826,   827,   835,   841,   845,   848,
     849,   853,   856,   857,   860,   869,   870,   873,   874,   877,
     883,   889,   890,   893,   894,   897,   907,   916,   922,   926,
     927,   930,   931,   934,   939,   946,   947,   948,   952,   956,
     959,   960,   963,   964,   968,   969,   973,   974,   975,   979,
     981,   983,   987,   988,   989,   990,   998,  1000,  1002,  1007,
    1009,  1014,  1015,  1020,  1021,  1022,  1023,  1028,  1037,  1039,
    1040,  1045,  1047,  1051,  1052,  1059,  1060,  1061,  1062,  1063,
    1068,  1076,  1077,  1080,  1081,  1084,  1091,  1092,  1097,  1098,
    1102,  1103,  1104,  1105,  1106,  1110,  1111,  1112,  1115,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1130,
    1137,  1139,  1145,  1146,  1147
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "aIDENTIFIER", "aPRAGMA", "aKNOWNTYPE",
  "aNUM", "aHEXNUM", "aDOUBLE", "aSTRING", "aWSTRING", "aSQSTRING",
  "aUUID", "aEOF", "SHL", "SHR", "MEMBERPTR", "EQUALITY", "INEQUALITY",
  "GREATEREQUAL", "LESSEQUAL", "LOGICALOR", "LOGICALAND", "ELLIPSIS",
  "tAGGREGATABLE", "tALLOCATE", "tANNOTATION", "tAPPOBJECT", "tASYNC",
  "tASYNCUUID", "tAUTOHANDLE", "tBINDABLE", "tBOOLEAN", "tBROADCAST",
  "tBYTE", "tBYTECOUNT", "tCALLAS", "tCALLBACK", "tCASE", "tCDECL",
  "tCHAR", "tCOCLASS", "tCODE", "tCOMMSTATUS", "tCONST", "tCONTEXTHANDLE",
  "tCONTEXTHANDLENOSERIALIZE", "tCONTEXTHANDLESERIALIZE", "tCONTROL",
  "tCPPQUOTE", "tDECODE", "tDEFAULT", "tDEFAULTBIND", "tDEFAULTCOLLELEM",
  "tDEFAULTVALUE", "tDEFAULTVTABLE", "tDISABLECONSISTENCYCHECK",
  "tDISPLAYBIND", "tDISPINTERFACE", "tDLLNAME", "tDOUBLE", "tDUAL",
  "tENABLEALLOCATE", "tENCODE", "tENDPOINT", "tENTRY", "tENUM",
  "tERRORSTATUST", "tEXPLICITHANDLE", "tEXTERN", "tFALSE", "tFASTCALL",
  "tFAULTSTATUS", "tFLOAT", "tFORCEALLOCATE", "tHANDLE", "tHANDLET",
  "tHELPCONTEXT", "tHELPFILE", "tHELPSTRING", "tHELPSTRINGCONTEXT",
  "tHELPSTRINGDLL", "tHIDDEN", "tHYPER", "tID", "tIDEMPOTENT", "tIGNORE",
  "tIIDIS", "tIMMEDIATEBIND", "tIMPLICITHANDLE", "tIMPORT", "tIMPORTLIB",
  "tIN", "tIN_LINE", "tINLINE", "tINPUTSYNC", "tINT", "tINT3264", "tINT64",
  "tINTERFACE", "tLCID", "tLENGTHIS", "tLIBRARY", "tLICENSED", "tLOCAL",
  "tLONG", "tMAYBE", "tMESSAGE", "tMETHODS", "tMODULE", "tNAMESPACE",
  "tNOCODE", "tNONBROWSABLE", "tNONCREATABLE", "tNONEXTENSIBLE", "tNOTIFY",
  "tNOTIFYFLAG", "tNULL", "tOBJECT", "tODL", "tOLEAUTOMATION", "tOPTIMIZE",
  "tOPTIONAL", "tOUT", "tPARTIALIGNORE", "tPASCAL", "tPOINTERDEFAULT",
  "tPRAGMA_WARNING", "tPROGID", "tPROPERTIES", "tPROPGET", "tPROPPUT",
  "tPROPPUTREF", "tPROXY", "tPTR", "tPUBLIC", "tRANGE", "tREADONLY",
  "tREF", "tREGISTER", "tREPRESENTAS", "tREQUESTEDIT", "tRESTRICTED",
  "tRETVAL", "tSAFEARRAY", "tSHORT", "tSIGNED", "tSIZEIS", "tSIZEOF",
  "tSMALL", "tSOURCE", "tSTATIC", "tSTDCALL", "tSTRICTCONTEXTHANDLE",
  "tSTRING", "tSTRUCT", "tSWITCH", "tSWITCHIS", "tSWITCHTYPE",
  "tTHREADING", "tTRANSMITAS", "tTRUE", "tTYPEDEF", "tUIDEFAULT", "tUNION",
  "tUNIQUE", "tUNSIGNED", "tUSESGETLASTERROR", "tUSERMARSHAL", "tUUID",
  "tV1ENUM", "tVARARG", "tVERSION", "tVIPROGID", "tVOID", "tWCHAR",
  "tWIREMARSHAL", "tAPARTMENT", "tNEUTRAL", "tSINGLE", "tFREE", "tBOTH",
  "','", "'?'", "':'", "'|'", "'^'", "'&'", "'<'", "'>'", "'-'", "'+'",
  "'*'", "'/'", "'%'", "'!'", "'~'", "CAST", "PPTR", "POS", "NEG",
  "ADDRESSOF", "'.'", "'['", "']'", "'{'", "'}'", "';'", "'('", "')'",
  "'='", "$accept", "input", "gbl_statements", "$@1", "imp_statements",
  "$@2", "int_statements", "semicolon_opt", "statement", "pragma_warning",
  "warnings", "typedecl", "cppquote", "import_start", "import",
  "importlib", "libraryhdr", "library_start", "librarydef", "m_args",
  "arg_list", "args", "arg", "array", "m_attributes", "attributes",
  "attrib_list", "str_list", "attribute", "uuid_string", "callconv",
  "cases", "case", "enums", "enum_list", "enum", "enumdef", "m_exprs",
  "m_expr", "expr", "expr_list_int_const", "expr_int_const", "expr_const",
  "fields", "field", "ne_union_field", "ne_union_fields", "union_field",
  "s_field", "funcdef", "declaration", "m_ident", "t_ident", "ident",
  "base_type", "m_int", "int_std", "coclass", "coclasshdr", "coclassdef",
  "namespacedef", "coclass_ints", "coclass_int", "dispinterface",
  "dispinterfacehdr", "dispint_props", "dispint_meths", "dispinterfacedef",
  "inherit", "interface", "interfacehdr", "interfacedef", "interfacedec",
  "module", "modulehdr", "moduledef", "storage_cls_spec",
  "function_specifier", "type_qualifier", "m_type_qual_list", "decl_spec",
  "m_decl_spec_no_type", "decl_spec_no_type", "declarator",
  "direct_declarator", "abstract_declarator",
  "abstract_declarator_no_direct", "m_abstract_declarator",
  "abstract_direct_declarator", "any_declarator",
  "any_declarator_no_direct", "m_any_declarator", "any_direct_declarator",
  "declarator_list", "m_bitfield", "struct_declarator",
  "struct_declarator_list", "init_declarator", "threading_type",
  "pointer_type", "structdef", "type", "typedef", "uniondef", "version", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,    44,    63,    58,   124,    94,    38,    60,    62,
      45,    43,    42,    47,    37,    33,   126,   437,   438,   439,
     440,   441,    46,    91,    93,   123,   125,    59,    40,    41,
      61
};
# endif

#define YYPACT_NINF -521

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-521)))

#define YYTABLE_NINF -260

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -521,    89,  1646,  -521,  -521,  -521,  -521,  -521,  -521,   156,
    -521,  -136,   172,  -521,   181,  -521,  -521,  -521,  -521,    -6,
      87,  -521,  -521,  -521,  -521,   218,    -6,   105,   -99,  -521,
     -75,    -6,   384,  -521,  -521,   223,   255,   384,  -521,  -521,
    2819,  -521,  -521,   -39,  -521,  -521,  -521,  -521,  -521,    25,
    2519,   -19,   -15,  -521,  -521,   -11,     6,  -521,    24,     3,
      27,    15,    29,    21,  -521,  -521,    66,  -521,    44,    44,
      44,   162,  2667,    79,    44,   106,   109,  -521,  -521,   208,
    -521,  -521,    94,  -521,    74,  -521,  -521,   119,  -521,  -521,
    -521,  -521,   279,  2667,  -521,  -521,    97,   107,  -127,  -120,
    -521,  -521,   122,  -521,  -521,   125,  -521,  -521,  -521,   126,
     131,  -521,  -521,  -521,  -521,  -521,  -521,  -521,  -521,  -521,
    -521,   155,  -521,  -521,  -521,   160,  -521,  -521,  -521,   161,
     173,  -521,  -521,  -521,  -521,   179,   180,   183,   184,   192,
    -521,   193,  -521,  -521,   195,  -521,   196,  -521,  -521,   197,
     205,  -521,  -521,  -521,  -521,  -521,  -521,  -521,  -521,  -521,
    -521,  -521,  -521,  -521,   211,  -521,  -521,  -521,   212,   213,
    -521,  -521,  -521,  -521,  -521,  -521,   214,  -521,  -521,   215,
    -521,  -521,  -521,   217,  -521,  -521,  -521,   222,   246,   247,
     251,  -521,  -521,  -521,   252,   260,  -521,  -521,   261,   263,
     264,  -121,  -521,  -521,  -521,  1528,   791,   177,   319,   322,
     337,   343,   347,   207,   182,  -521,  -521,  -521,  -521,   162,
     209,   268,  -521,  -521,  -521,  -521,  -521,   -55,  -521,  -521,
    -521,   350,   271,  -521,  -521,  -521,  -521,  -521,  -521,  -521,
    -521,  -521,  -521,  -521,  -521,   162,   162,  -521,   267,   -96,
    -521,  -521,  -521,    44,  -521,  -521,  -521,   269,   355,  -521,
     289,   270,  -521,   276,  -521,   477,    61,   355,   646,   646,
     478,   479,   646,   646,   482,   491,   646,   493,   646,   646,
    2080,   646,   646,   494,   -78,   497,   646,  2667,   646,   646,
    2667,   194,  2667,  2667,    61,    75,   501,  2667,  2819,   308,
    -521,   309,  -521,  -521,  -521,   311,  -521,   314,  -521,  -521,
    -521,    15,  2593,  -521,   326,  -521,  -521,  -521,  -521,   326,
    -102,  -521,  -521,   -37,  -521,   332,   -66,   320,   329,  -521,
    -521,  1130,    72,   327,  -521,   646,   266,  2080,  -521,  -521,
    -521,   333,   353,  -521,   328,   534,  -521,   -30,   177,   -21,
     335,  -521,  -521,   336,   341,  -521,  -521,  -521,  -521,  -521,
    -521,  -521,  -521,  -521,   339,  -521,   646,   646,   646,   646,
     646,   646,   575,  2328,  -143,  -521,  2328,   342,   344,  -521,
    -123,   346,   348,   349,   351,   352,   354,   357,  1334,   358,
    2593,   159,   359,  -118,  -521,  2328,   362,   363,   364,   367,
     365,  -114,  2001,   368,  -521,  -521,  -521,  -521,  -521,   370,
     378,   379,   380,   374,  -521,   381,   382,   383,  -521,  2819,
     543,  -521,  -521,  -521,   162,    15,   -23,  -521,  1027,  -521,
     375,  2593,   387,  1410,   389,   474,  1233,    15,  -521,  2593,
    -521,  -521,  -521,  -521,   594,  -521,  2242,   390,   414,  -521,
    -521,  -521,   355,   646,  -521,    18,  -521,  2593,  -521,   397,
    -521,   391,  -521,   401,  -521,  -521,  -521,  2593,    12,    12,
      12,    12,    12,    12,  2111,   338,   646,   646,   607,   646,
     646,   646,   646,   646,   646,   646,   646,   646,   646,   646,
     646,   646,   646,   646,   646,   646,   608,   646,   646,  -521,
    -521,  -521,   604,  -521,  -521,  -521,  -521,  -521,  -521,  -521,
    -521,  -521,  -521,   159,  -521,  1775,  -521,   159,  -521,  -521,
    -521,   -93,  -521,   646,  -521,  -521,  -521,  -521,   646,  -521,
    -521,  -521,  -521,  -521,  -521,  -521,  -521,   610,  -521,  -521,
    -521,  -521,   405,  -521,  -521,   435,  -521,  -521,  -521,  -521,
     162,   169,  -521,  -521,  2593,   413,  -521,  -521,  -521,    15,
    -521,  -521,  -521,  -521,  2006,  -521,  -521,  -521,  -521,   159,
     419,   355,  -521,  -521,   338,  -521,  -521,  1903,  -521,   338,
    -521,   420,   -58,   204,   204,  -521,   718,   718,   305,   305,
    2198,  2347,  2307,   103,   506,  2354,   305,   305,   115,   115,
      12,    12,    12,  -521,  2270,  -521,  -521,  -521,   340,  -521,
     421,   159,   423,  -521,  2080,  -521,  -521,   425,  -521,    15,
     909,   162,  -521,  -521,  1336,  -521,  -521,  -521,   444,  -521,
    -115,  -521,   431,  -521,   429,   498,  -521,   430,   159,   434,
    -521,   646,  2080,  -521,   646,  -521,  -521,   340,  -521,  -521,
    -521,   437,  -521,  -521,  -521,  -521,    15,   646,  -521,   159,
    -521,  -521,  -521,  -521,   340,  -521,  -521,  -521,    12,   438,
    2328,  -521,  -521,  -521,  -521,  -521,   -13,  -521,  -521,   646,
     475,  -521,  -521,   476,   -52,   -52,  -521,  -521,   454,  -521,
    -521
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       3,     0,     2,     1,    33,   380,   271,   263,   282,     0,
     319,     0,     0,   270,   258,   272,   315,   269,   273,   274,
       0,   318,   276,   283,   281,     0,   274,     0,     0,   317,
       0,   274,     0,   278,   316,   258,   258,   268,   379,   264,
      74,    12,    34,     0,    28,    13,    31,    13,    11,     0,
      67,   382,     0,   381,   265,     0,     0,     9,     0,     0,
       0,    26,     0,   301,     7,     6,     0,    10,   324,   324,
     324,     0,     0,   384,   324,     0,   386,   284,   285,     0,
     292,   293,   383,   260,     0,   275,   280,     0,   303,   304,
     279,   288,     0,     0,   277,   266,   385,     0,   387,     0,
     267,    75,     0,    77,    78,     0,    79,    80,    81,     0,
       0,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,     0,    95,    96,    97,     0,    99,   100,   101,     0,
       0,   104,   105,   106,   107,     0,     0,     0,     0,     0,
     113,     0,   115,   116,     0,   118,     0,   120,   121,   124,
       0,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,     0,   139,   140,   141,     0,     0,
     144,   145,   146,   147,   377,   148,     0,   150,   375,     0,
     152,   153,   154,     0,   156,   157,   158,     0,     0,     0,
       0,   163,   376,   164,     0,     0,   168,   169,     0,     0,
       0,     0,    69,   173,    29,    66,    66,    66,   258,     0,
       0,   258,   258,     0,   382,   286,   294,   305,   313,     0,
     384,   386,    30,     8,   289,     4,   310,     0,    27,   308,
     309,     0,     0,    24,   328,   325,   327,   326,   261,   262,
     176,   177,   178,   179,   320,     0,     0,   332,   368,   331,
     255,   382,   384,   324,   386,   322,    32,     0,   184,    48,
       0,     0,   241,     0,   247,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   194,     0,     0,     0,     0,     0,   194,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    68,
      49,     0,    21,    22,    23,     0,    19,     0,    17,    14,
      20,    26,     0,    67,   383,    51,    52,   311,   312,   385,
     387,    53,   254,    66,     3,     0,    66,     0,     0,   302,
      24,    66,     0,     0,   330,     0,     0,    55,   334,   323,
      47,     0,   186,   187,   190,     0,   388,    66,    66,    66,
       0,   175,   174,     0,     0,   205,   196,   197,   198,   202,
     203,   204,   199,   200,     0,   201,     0,     0,     0,     0,
       0,     0,     0,   239,     0,   237,   240,     0,     0,    72,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   353,     0,     0,   192,   195,     0,     0,     0,     0,
       0,     0,     0,     0,   370,   371,   372,   373,   374,     0,
       0,     0,     0,   392,   394,     0,     0,     0,    70,    74,
       0,    18,    15,    54,     0,    26,     0,   290,    66,   295,
       0,     0,     0,     0,     0,     0,    66,    26,    25,    67,
     321,   329,   333,   369,     0,    65,     0,     0,    59,    56,
      57,   191,   185,     0,    36,     0,   378,     0,   242,     0,
     390,    67,   248,     0,    76,   167,    82,     0,   229,   228,
     227,   230,   225,   226,     0,   341,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
      94,    98,     0,   102,   103,   108,   109,   110,   111,   112,
     114,   117,   119,   353,   320,    55,   358,   353,   355,   354,
      62,   350,   123,   194,   122,   138,   142,   143,     0,   151,
     155,   159,   160,   162,   161,   165,   166,     0,   170,   171,
     172,    71,     0,    13,   361,   389,   287,   291,     5,   297,
       0,   384,   296,   299,     0,     0,   253,   300,    24,    26,
     314,    64,    63,   335,     0,   188,   189,    37,    35,     0,
     386,   256,   246,   245,   341,   236,   320,    55,   345,   341,
     342,     0,   338,   218,   219,   231,   212,   213,   216,   217,
     207,   208,     0,   209,   210,   211,   215,   214,   221,   220,
     223,   224,   222,   232,     0,   238,    73,    61,   353,   320,
       0,   353,     0,   349,    55,   357,   193,     0,   393,    26,
      66,     0,   251,   298,    66,   306,    60,    58,   363,   366,
       0,   244,     0,   257,     0,   341,   320,     0,   353,     0,
     337,     0,    55,   344,     0,   235,   348,   353,   359,   352,
     356,     0,   149,    50,    16,   362,    26,     0,   365,     0,
     243,   180,   234,   336,   353,   346,   340,   343,   233,     0,
     206,   351,   360,   307,   364,   367,     0,   339,   347,     0,
       0,   391,   181,     0,    66,    66,   250,   183,     0,   182,
     249
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -521,  -521,   360,  -521,   -42,  -521,  -304,  -291,     0,  -521,
    -521,  -521,  -521,  -521,   227,  -521,  -521,  -521,    10,  -472,
    -521,  -521,  -266,  -218,  -191,    -2,  -521,  -521,  -275,   369,
     -65,  -521,  -521,  -521,  -521,   216,    13,   377,   143,  -242,
    -521,  -228,  -264,  -521,  -521,  -521,  -521,   -18,  -177,  -521,
     237,  -521,    -3,   -67,  -521,   110,   116,     5,  -521,    11,
      17,  -521,  -521,   624,  -521,  -521,  -521,  -521,  -521,   -33,
    -521,    19,    16,  -521,  -521,    20,  -521,  -521,  -298,  -462,
     -49,    32,    30,  -235,  -521,  -521,  -521,  -495,  -521,  -520,
    -521,  -448,  -521,  -521,  -521,    22,  -521,   456,  -521,   392,
       1,   -31,  -521,     7,  -521
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,   324,   205,   543,   331,   229,   302,    42,
     455,    43,    44,    45,    46,   303,   213,    47,   304,   447,
     448,   449,   450,   516,    49,   313,   201,   380,   202,   353,
     517,   676,   682,   341,   342,   343,   251,   393,   394,   373,
     374,   375,   377,   347,   458,   462,   349,   687,   688,   555,
      52,   632,    84,   518,    53,    86,    54,   305,    56,   306,
     307,   323,   427,    59,    60,   326,   433,    61,   232,    62,
      63,   308,   309,   218,    66,   310,    68,    69,    70,   332,
      71,   234,    72,   248,   249,   580,   639,   581,   582,   519,
     612,   520,   521,   545,   658,   629,   630,   250,   409,   203,
     252,    74,    75,   254,   415
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      50,   219,    41,    73,   247,   206,   246,    55,   381,    76,
     333,   334,    48,    57,   389,    51,   312,   217,    65,    58,
     423,    64,    67,   418,   567,   679,   436,   376,   478,  -259,
     376,   338,    97,    99,   440,    12,   263,   388,   680,   498,
     395,   253,   430,   610,    25,   382,   395,   402,   385,   628,
     387,   220,   608,   392,  -259,   215,   174,   221,   399,   502,
     178,   298,   261,   214,   523,   607,   499,   659,   523,   613,
     351,   443,    79,   352,   325,   238,    25,   239,  -259,   634,
     -43,   413,   414,   299,   640,   264,   503,   192,    10,     3,
      85,   524,   660,   376,   446,   530,    87,   441,   235,   235,
     235,   236,   237,  -259,   235,   637,   255,   336,    91,    92,
     336,   240,   337,    16,   635,   614,    10,   476,   477,   478,
     479,   480,   481,   482,   468,   469,   470,   471,   472,   473,
     474,   478,   426,    93,   546,   431,    90,    40,    21,   628,
     663,    94,   651,   241,   541,   336,   560,   647,    95,   432,
     642,    40,   247,   100,   246,   686,   457,   431,   431,    77,
     646,    78,   238,   649,   239,   238,    40,   239,   204,   425,
     669,   459,   463,    40,   664,    80,   456,    81,   247,   247,
     246,   246,    40,    29,    82,   460,    83,   207,   -38,   544,
     666,   344,   222,   681,   327,    34,   223,   242,   240,   671,
     354,   240,   471,    50,    50,   231,    73,    73,    97,    99,
     226,   224,    76,    76,   496,   497,   677,   257,    51,    51,
     478,    88,   228,    89,   243,   566,    96,   568,    83,   225,
     241,   391,   227,   241,   583,   584,   230,   586,   587,   588,
     589,   590,   591,   592,   593,   594,   595,   596,   597,   598,
     599,   600,   601,   602,   624,   604,   400,   578,    98,   403,
      83,   410,   411,   424,   244,   247,   417,   246,   625,   355,
     605,   233,   356,   357,   358,   359,   360,   361,   390,   258,
     245,   395,   260,   235,   242,   339,   -40,   242,   391,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   627,  -259,
     617,   -39,  -259,   615,   -41,   496,   497,   493,   494,   495,
     440,   243,   262,   256,   243,   622,   -42,   496,   497,   476,
     477,   478,   314,   475,    83,   315,   259,   316,   653,   439,
     265,   438,    73,   266,   267,   390,   362,   440,    76,   268,
     317,   513,   318,   238,    51,   239,   319,   461,    83,   440,
     320,   514,    83,   328,   244,   329,   578,   247,   238,   246,
     239,   578,   336,   269,   643,   673,   440,   515,   270,   271,
     245,   404,   405,   406,   407,   408,  -252,   240,  -252,   240,
      40,   272,   550,   363,    10,   344,   655,   273,   274,   -44,
     219,   275,   276,   674,   491,   492,   493,   494,   495,   668,
     277,   278,   670,   279,   280,   281,   496,   497,   569,   241,
     579,   241,   321,   282,   364,   376,   -45,   578,   574,   283,
     284,   285,   286,   287,     8,   288,    50,   365,    41,    73,
     289,   554,   551,    55,   439,    76,   438,    73,    48,    57,
     220,    51,   547,    76,    65,    58,   221,    64,    67,    51,
     611,   683,   214,   366,   290,   291,   367,   368,   444,   292,
     293,   370,   371,   242,   570,   242,   391,    19,   294,   295,
     445,   296,   297,   345,   372,   -46,   330,   335,   340,   346,
      22,    23,    24,   247,   348,   246,   350,   378,   379,    26,
     243,   383,   243,   431,   431,   491,   492,   493,   494,   495,
     384,   620,   386,   396,   633,   219,   398,   496,   497,   579,
     416,   419,   638,   390,   579,   391,   429,   420,   421,   422,
     476,   477,   478,   479,   480,   481,   482,   434,   391,    31,
     576,  -259,   514,    33,   435,   452,   442,   240,   453,   451,
     454,   336,    10,   336,   464,   465,   577,   467,   515,   528,
     466,   500,   542,   501,   247,   504,   246,   505,   506,   549,
     507,   508,   390,   509,    20,   391,   510,   512,   522,   241,
     579,   525,   526,   527,   529,   390,   537,   532,   355,   533,
       5,   356,   357,   358,   359,   360,   361,   534,   535,   536,
     538,   539,   540,   391,   552,   557,   564,   355,   572,   563,
     356,   357,   358,   359,   360,   361,   571,     6,   573,     7,
     585,   603,   390,   606,   619,     8,   618,   621,    50,    10,
     623,    73,   439,   242,   438,    73,   631,    76,   657,   641,
     648,    76,   650,    51,   652,    13,   661,    51,   662,   665,
     390,   208,    15,   667,    16,   362,   672,   678,    17,   355,
     243,    18,   356,   357,   358,   359,   360,   361,    19,   684,
     685,   690,   558,   412,   362,   401,   616,   689,   565,    21,
     556,    22,    23,    24,   216,   322,   397,     0,     0,     0,
      26,   675,     0,     0,   428,     0,     0,     0,     0,     0,
     576,     0,   363,   488,   489,   490,   491,   492,   493,   494,
     495,   336,     0,     0,     0,     0,   577,     0,   496,   497,
       0,   363,     0,     0,    29,     0,   362,     0,     0,    30,
      31,    32,     0,   364,    33,     0,    34,     0,     0,     0,
     211,     0,   476,   477,   478,     0,   365,   481,   482,   212,
       0,    37,   364,     0,     0,     0,     0,     0,     0,    38,
      39,     0,     0,     0,     0,   365,     0,     0,     0,     0,
       0,     0,   366,   363,     0,   367,   368,   369,     0,     0,
     370,   371,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   366,     0,   372,   367,   368,   369,     0,     0,   370,
     371,     0,     0,     0,   364,     4,     5,     0,   561,     0,
       0,     0,   372,     0,     0,     0,     0,   365,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     6,     0,     7,     0,     0,     0,     0,
       0,     8,     9,   366,     0,    10,   367,   368,   369,     0,
      11,   370,   371,     0,     0,     0,     0,     0,     0,    12,
       0,    13,     0,     0,   372,     0,     0,    14,    15,     0,
      16,     0,     0,     0,    17,     0,     0,    18,     0,     0,
       0,     0,     0,     0,    19,     0,     0,     0,     0,     0,
       0,    20,   301,     0,     0,    21,     0,    22,    23,    24,
      25,     0,     0,     0,     0,     0,    26,     0,     0,     0,
       0,    27,     0,     0,     0,     0,   489,   490,   491,   492,
     493,   494,   495,     4,     5,     0,     0,     0,    28,     0,
     496,   497,     0,     0,     0,     0,     0,     0,     0,     0,
      29,     0,     0,     0,     0,    30,    31,    32,     0,     0,
      33,     6,    34,     7,     0,     0,    35,     0,     0,     8,
       9,     0,     0,    10,     0,    36,     0,    37,    11,     0,
       0,     0,     0,     0,     0,    38,    39,    12,     0,    13,
       0,     0,     0,     0,     0,    14,    15,     0,    16,     0,
       0,     0,    17,     0,     0,    18,     0,     0,     0,     0,
       0,     0,    19,     0,    40,     0,     0,   311,     0,    20,
     301,     0,     0,    21,     0,    22,    23,    24,    25,     0,
       0,     0,     0,     0,    26,     0,     0,     0,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     5,     0,     0,     0,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    29,     0,
       0,     0,     0,    30,    31,    32,     0,     0,    33,     6,
      34,     7,     0,     0,    35,     0,     0,     8,     9,     0,
       0,    10,     0,    36,     0,    37,    11,     0,     0,     0,
       0,     0,     0,    38,    39,    12,     0,    13,     0,     0,
       0,     0,     0,    14,    15,     0,    16,     0,     0,     0,
      17,     0,     0,    18,     0,     0,     0,     0,     0,     0,
      19,     0,    40,     0,     0,   654,     0,    20,     0,     0,
       0,    21,     0,    22,    23,    24,    25,     0,     0,     0,
       0,     0,    26,     0,     4,     5,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    28,     0,     0,     0,     0,     0,
       0,     0,     6,     0,     7,     0,    29,     0,     0,     0,
       8,    30,    31,    32,    10,     0,    33,     0,    34,    11,
       0,     0,    35,     0,     0,     0,     0,     0,     0,     0,
      13,    36,     0,    37,     0,     0,    14,    15,     0,    16,
       0,    38,    39,    17,     0,     0,    18,     0,     0,     0,
       0,     0,     0,    19,     0,     0,     0,     0,     0,     0,
      20,     0,     0,     0,    21,     0,    22,    23,    24,     0,
      40,     0,     0,   548,     0,    26,     0,     4,     5,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    28,     0,     0,
       0,     0,     0,     0,     0,     6,     0,     7,     0,    29,
       0,     0,     0,     8,    30,    31,    32,    10,     0,    33,
       0,    34,    11,     0,     0,    35,     0,     0,     0,     0,
       0,     0,     0,    13,    36,     0,    37,     0,     0,    14,
      15,     0,    16,     0,    38,    39,    17,     0,     0,    18,
       0,     0,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,     0,    20,     0,     0,     0,    21,     0,    22,
      23,    24,     0,    40,     0,     0,   437,     0,    26,     0,
       4,     5,     0,     0,     0,     0,     0,     0,   476,   477,
     478,   479,   480,   481,   482,   483,   484,     0,     0,     0,
      28,     0,     0,     0,     0,     0,     0,     0,     6,     0,
       7,     0,    29,     0,     0,     0,     8,    30,    31,    32,
      10,     0,    33,     0,    34,    11,     0,     0,    35,     0,
       0,     0,     0,     0,     0,     0,    13,    36,     0,    37,
       0,     0,    14,    15,     0,    16,     0,    38,    39,    17,
       0,     0,    18,     0,     0,     5,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,    20,     0,     0,     0,
      21,     0,    22,    23,    24,     0,    40,     0,     0,   559,
       0,    26,     6,     0,     7,     0,     0,     0,     0,     0,
       8,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,    28,     0,     0,     0,     0,     0,     0,
      13,     0,     0,     0,     0,    29,   208,    15,     0,    16,
      30,    31,    32,    17,     0,    33,    18,    34,     0,     0,
       0,    35,     0,    19,     0,     0,     0,     0,     0,     0,
      36,     0,    37,     0,    21,     0,    22,    23,    24,     0,
      38,    39,     0,     0,     0,    26,     0,   485,     0,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,     0,
       0,     0,     4,     5,     0,     0,   496,   497,     0,    40,
       0,   300,   656,   511,     0,     0,     0,     0,     0,    29,
       0,     0,     0,     0,    30,    31,    32,     0,     0,    33,
       6,    34,     7,     0,     0,   211,     0,     0,     8,     9,
       0,     0,    10,     0,   212,     0,    37,    11,     0,     0,
       0,     0,     0,     0,    38,    39,    12,     0,    13,     0,
       0,     0,     0,     0,    14,    15,     0,    16,     0,     0,
       0,    17,     0,     0,    18,     0,     0,     0,     0,     0,
       0,    19,     0,    40,     0,     0,   553,     0,    20,   301,
       0,     0,    21,     0,    22,    23,    24,    25,     0,     0,
       0,     0,     0,    26,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     0,     0,     0,    28,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    29,     0,     0,
       0,     0,    30,    31,    32,     0,     0,    33,     6,    34,
       7,     0,     0,    35,     0,     0,     8,     9,     0,     0,
      10,     0,    36,     0,    37,    11,     0,     0,     0,     0,
       0,     0,    38,    39,    12,     0,    13,     0,     0,     0,
       0,     0,    14,    15,     0,    16,     0,     0,     0,    17,
       0,     0,    18,     0,     0,     0,     0,     0,     0,    19,
       0,    40,     0,     0,     0,     0,    20,     0,     0,     0,
      21,     0,    22,    23,    24,    25,     0,     0,     0,     0,
       0,    26,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    28,     0,     0,     0,     0,     0,     0,
       5,     0,     0,     0,     0,    29,     0,     0,     0,     0,
      30,    31,    32,     0,     0,    33,     0,    34,     0,     0,
       0,    35,     0,     0,     0,     0,     0,     6,   -66,     7,
      36,     0,    37,     0,   240,     8,     0,     0,     0,    10,
      38,    39,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,     0,     0,     0,     0,
       0,   208,    15,     0,    16,     0,   241,     0,    17,    40,
       0,    18,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,    22,    23,    24,     0,     0,     0,     0,     0,     0,
      26,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     242,     0,     0,     0,     0,     0,     0,     0,     5,     0,
       0,     0,     0,     0,    29,     0,     0,     0,     0,    30,
      31,    32,     0,     0,    33,     0,    34,   243,     0,     0,
     211,     0,     0,     0,     0,     6,     0,     7,     0,   212,
       0,    37,   240,     8,     0,     0,     0,    10,     0,    38,
      39,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,     0,     0,     0,   609,     0,   208,
      15,     0,    16,     0,   241,     0,    17,     0,    40,    18,
       0,     0,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,    22,
      23,    24,     0,     0,     0,     0,     0,     0,    26,     0,
       0,     5,     0,     0,     0,   476,   477,   478,   479,   480,
     481,   482,   483,   484,     0,     0,     0,     0,   242,   626,
       0,     0,     0,     0,     0,     0,     0,     0,     6,     0,
       7,     0,    29,     0,     0,     0,     8,    30,    31,    32,
      10,     0,    33,     0,    34,   243,     0,     0,   211,     0,
       0,     0,     0,     0,     0,     0,    13,   212,     0,    37,
       0,     0,   208,    15,     0,    16,     0,    38,    39,    17,
       0,     0,    18,     0,     0,     5,     0,     0,     0,    19,
       0,     0,     0,     0,     0,   636,     0,     0,     0,     0,
      21,     0,    22,    23,    24,     0,    40,     0,     0,     0,
       0,    26,     6,     0,     7,     0,     0,     0,     0,     0,
       8,     0,     0,     0,    10,   476,   477,   478,   479,   480,
     481,   482,   483,   484,     0,     0,     0,     0,     0,     0,
      13,     0,     0,     0,     0,    29,   208,    15,     0,    16,
      30,    31,    32,    17,     0,    33,    18,    34,     0,     0,
       0,   211,     0,    19,     0,     0,     0,     0,     0,     0,
     212,     0,    37,     0,    21,     0,    22,    23,    24,     0,
      38,    39,     0,     0,   485,    26,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,     0,     0,     0,     0,
       0,     0,     0,   496,   497,     0,     0,     0,     0,    40,
     531,     0,   476,   477,   478,   479,   480,   481,   482,    29,
     484,     0,     0,     0,    30,    31,    32,     0,     0,    33,
       0,    34,     0,     0,     0,   211,     0,     0,     0,     0,
       0,     0,     0,     0,   212,     0,    37,     0,     0,     0,
       0,     0,     0,     0,    38,    39,   476,   477,   478,   479,
     480,   481,   482,   483,   484,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    40,   476,   477,   478,   479,   480,   481,
     482,   483,   484,     0,   485,     0,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,     0,     0,     0,     0,
       0,     0,     0,   496,   497,     0,     0,     0,     0,     0,
     575,   476,   477,   478,   479,   480,   481,   482,   483,   484,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   476,   477,   478,   479,   480,   481,   482,   483,
     484,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   476,   477,   478,   479,   480,   481,   482,   476,   477,
     478,   479,   480,   481,   482,     0,     0,     0,     0,     0,
       0,     0,     0,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,     0,     0,     0,     0,     0,     0,     0,
     496,   497,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   485,     0,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,     0,     0,     0,
       0,     0,     0,     0,   496,   497,   562,     0,     0,     0,
       0,     0,     0,   485,     0,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,     0,     0,     0,     0,     0,
       0,     0,   496,   497,   645,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     485,   644,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,     0,     0,     0,     0,     0,     0,     0,   496,
     497,   485,     0,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,     0,     5,     0,     0,     0,     0,     0,
     496,   497,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   489,   490,   491,   492,   493,   494,   495,   496,
     497,     6,     0,     7,     0,     0,   496,   497,     0,     8,
       9,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,     0,    13,
       0,     0,     0,     0,     0,   208,    15,     0,    16,     0,
       0,     0,    17,     0,     0,    18,     0,     0,     5,     0,
       0,     0,    19,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,    22,    23,    24,    25,     0,
       0,   209,     0,     0,    26,     6,     0,     7,   210,     0,
       0,     0,     0,     8,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,     0,     0,     0,     0,    29,   208,
      15,     0,    16,    30,    31,    32,    17,     0,    33,    18,
      34,     0,     5,     0,   211,     0,    19,     0,     0,     0,
       0,     0,     0,   212,     0,    37,     0,    21,     0,    22,
      23,    24,     0,    38,    39,     0,     0,     0,    26,     6,
       0,     7,     0,     0,     0,     0,     0,     8,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,     0,     0,
       0,     0,    29,   208,    15,     0,     0,    30,    31,    32,
      17,     0,    33,    18,    34,     0,     0,     0,   211,     0,
      19,     0,     0,     0,     0,     0,     0,   212,     0,    37,
       0,     0,     0,    22,    23,    24,     0,    38,    39,     0,
       0,     0,    26,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    30,    31,    32,     0,     0,    33,     0,     0,     0,
       0,     0,   211,     0,     0,     0,     0,     0,     0,     0,
       0,   212,     0,    37,     0,     0,     0,     0,     0,     0,
       0,    38,    39,   101,     0,   102,   103,   104,   105,   106,
     107,     0,   108,     0,     0,   109,     0,   110,     0,     0,
       0,   111,   112,     0,   113,   114,   115,   116,     0,   117,
     118,   119,   120,   121,   122,   123,   124,     0,   125,     0,
     126,   127,   128,   129,   130,     0,     0,   131,     0,     0,
       0,   132,     0,   133,   134,     0,   135,   136,   137,   138,
     139,   140,     0,   141,   142,   143,   144,   145,   146,     0,
       0,   147,     0,     0,   148,     0,     0,     0,     0,   149,
     150,     0,   151,   152,     0,   153,   154,     0,     0,     0,
     155,   156,   157,   158,   159,   160,     0,   161,   162,   163,
     164,   165,   166,   167,     0,   168,     0,   169,     0,   170,
     171,   172,   173,   174,   175,   176,   177,   178,     0,   179,
     180,   181,   182,     0,     0,     0,   183,     0,     0,   184,
       0,     0,   185,   186,     0,     0,   187,   188,   189,   190,
       0,     0,   191,     0,   192,     0,   193,   194,   195,   196,
     197,   198,   199,     0,     0,   200
};

static const yytype_int16 yycheck[] =
{
       2,    50,     2,     2,    71,    47,    71,     2,   272,     2,
     245,   246,     2,     2,   280,     2,   207,    50,     2,     2,
     311,     2,     2,   298,     6,    38,   330,   269,    16,   156,
     272,   249,    35,    36,   332,    58,   156,   279,    51,   182,
     282,    72,   108,   515,    99,   273,   288,   289,   276,   569,
     278,    50,   514,   281,   156,    50,   134,    50,   286,   182,
     138,   182,    93,    50,   182,   513,   209,   182,   182,   517,
       9,   335,   208,    12,   129,     3,    99,     5,   205,   574,
     207,     6,     7,   204,   579,   205,   209,   165,    44,     0,
      96,   209,   207,   335,   336,   209,     9,   332,    68,    69,
      70,    69,    70,   205,    74,   577,    74,   203,     3,   208,
     203,    39,   208,    69,   576,   208,    44,    14,    15,    16,
      17,    18,    19,    20,   366,   367,   368,   369,   370,   371,
     372,    16,   323,   208,   425,   326,    26,   203,    94,   659,
     635,    31,   614,    71,   419,   203,   437,   609,    32,   326,
     208,   203,   219,    37,   219,   207,   347,   348,   349,     3,
     608,     5,     3,   611,     5,     3,   203,     5,   207,   206,
     642,   348,   349,   203,   636,     3,   206,     5,   245,   246,
     245,   246,   203,   139,     3,   206,     5,   162,   207,   424,
     638,   258,   207,   206,   227,   151,   207,   125,    39,   647,
     267,    39,   444,   205,   206,   184,   205,   206,   211,   212,
     207,   205,   205,   206,   202,   203,   664,     9,   205,   206,
      16,     3,   207,     5,   152,   453,     3,   209,     5,   205,
      71,   280,   205,    71,   476,   477,   207,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   558,   497,   287,   475,     3,   290,
       5,   292,   293,   312,   192,   332,   297,   332,   559,     3,
     498,   205,     6,     7,     8,     9,    10,    11,   280,   205,
     208,   523,     3,   253,   125,   253,   207,   125,   337,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   564,   205,
     528,   207,   205,   521,   207,   202,   203,   192,   193,   194,
     608,   152,   205,   207,   152,   550,   207,   202,   203,    14,
      15,    16,     3,   372,     5,     3,   207,     5,   619,   331,
     208,   331,   331,   208,   208,   337,    70,   635,   331,   208,
       3,   390,     5,     3,   331,     5,     3,   349,     5,   647,
       3,   192,     5,     3,   192,     5,   574,   424,     3,   424,
       5,   579,   203,   208,   582,   656,   664,   208,   208,   208,
     208,   177,   178,   179,   180,   181,   207,    39,   209,    39,
     203,   208,   431,   117,    44,   452,   621,   208,   208,   207,
     439,   208,   208,   657,   190,   191,   192,   193,   194,   641,
     208,   208,   644,   208,   208,   208,   202,   203,   457,    71,
     475,    71,   205,   208,   148,   657,   207,   635,   467,   208,
     208,   208,   208,   208,    40,   208,   428,   161,   428,   428,
     208,   433,   431,   428,   436,   428,   436,   436,   428,   428,
     439,   428,   426,   436,   428,   428,   439,   428,   428,   436,
     515,   679,   439,   187,   208,   208,   190,   191,   192,   208,
     208,   195,   196,   125,   457,   125,   515,    83,   208,   208,
     204,   208,   208,   184,   208,   207,   205,   210,   209,   209,
      96,    97,    98,   550,   208,   550,     9,     9,     9,   105,
     152,     9,   152,   684,   685,   190,   191,   192,   193,   194,
       9,   543,     9,     9,   571,   554,     9,   202,   203,   574,
       9,   203,   577,   515,   579,   564,   184,   208,   207,   205,
      14,    15,    16,    17,    18,    19,    20,   207,   577,   145,
     192,   205,   192,   149,   205,   182,   209,    39,   210,   206,
       6,   203,    44,   203,   209,   209,   208,   208,   208,   182,
     209,   209,     9,   209,   621,   209,   621,   209,   209,   184,
     209,   209,   564,   209,    90,   614,   209,   209,   209,    71,
     635,   209,   209,   209,   209,   577,   202,   209,     3,   209,
       5,     6,     7,     8,     9,    10,    11,   209,   209,   209,
     209,   209,   209,   642,   207,   206,   182,     3,   207,   209,
       6,     7,     8,     9,    10,    11,   209,    32,   207,    34,
       3,     3,   614,     9,   209,    40,     6,   182,   620,    44,
     207,   620,   624,   125,   624,   624,   207,   620,   184,   209,
     209,   624,   209,   620,   209,    60,   205,   624,   209,   209,
     642,    66,    67,   209,    69,    70,   209,   209,    73,     3,
     152,    76,     6,     7,     8,     9,    10,    11,    83,   184,
     184,   207,   435,   294,    70,   288,   523,   685,   452,    94,
     433,    96,    97,    98,    50,   219,   284,    -1,    -1,    -1,
     105,   659,    -1,    -1,   324,    -1,    -1,    -1,    -1,    -1,
     192,    -1,   117,   187,   188,   189,   190,   191,   192,   193,
     194,   203,    -1,    -1,    -1,    -1,   208,    -1,   202,   203,
      -1,   117,    -1,    -1,   139,    -1,    70,    -1,    -1,   144,
     145,   146,    -1,   148,   149,    -1,   151,    -1,    -1,    -1,
     155,    -1,    14,    15,    16,    -1,   161,    19,    20,   164,
      -1,   166,   148,    -1,    -1,    -1,    -1,    -1,    -1,   174,
     175,    -1,    -1,    -1,    -1,   161,    -1,    -1,    -1,    -1,
      -1,    -1,   187,   117,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   187,    -1,   208,   190,   191,   192,    -1,    -1,   195,
     196,    -1,    -1,    -1,   148,     4,     5,    -1,   204,    -1,
      -1,    -1,   208,    -1,    -1,    -1,    -1,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    -1,    34,    -1,    -1,    -1,    -1,
      -1,    40,    41,   187,    -1,    44,   190,   191,   192,    -1,
      49,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    -1,   208,    -1,    -1,    66,    67,    -1,
      69,    -1,    -1,    -1,    73,    -1,    -1,    76,    -1,    -1,
      -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,
      -1,    90,    91,    -1,    -1,    94,    -1,    96,    97,    98,
      99,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,
      -1,   110,    -1,    -1,    -1,    -1,   188,   189,   190,   191,
     192,   193,   194,     4,     5,    -1,    -1,    -1,   127,    -1,
     202,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,    -1,    -1,
     149,    32,   151,    34,    -1,    -1,   155,    -1,    -1,    40,
      41,    -1,    -1,    44,    -1,   164,    -1,   166,    49,    -1,
      -1,    -1,    -1,    -1,    -1,   174,   175,    58,    -1,    60,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    69,    -1,
      -1,    -1,    73,    -1,    -1,    76,    -1,    -1,    -1,    -1,
      -1,    -1,    83,    -1,   203,    -1,    -1,   206,    -1,    90,
      91,    -1,    -1,    94,    -1,    96,    97,    98,    99,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,   110,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     4,     5,    -1,    -1,    -1,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,    -1,    -1,   149,    32,
     151,    34,    -1,    -1,   155,    -1,    -1,    40,    41,    -1,
      -1,    44,    -1,   164,    -1,   166,    49,    -1,    -1,    -1,
      -1,    -1,    -1,   174,   175,    58,    -1,    60,    -1,    -1,
      -1,    -1,    -1,    66,    67,    -1,    69,    -1,    -1,    -1,
      73,    -1,    -1,    76,    -1,    -1,    -1,    -1,    -1,    -1,
      83,    -1,   203,    -1,    -1,   206,    -1,    90,    -1,    -1,
      -1,    94,    -1,    96,    97,    98,    99,    -1,    -1,    -1,
      -1,    -1,   105,    -1,     4,     5,    -1,   110,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   127,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    34,    -1,   139,    -1,    -1,    -1,
      40,   144,   145,   146,    44,    -1,   149,    -1,   151,    49,
      -1,    -1,   155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,   164,    -1,   166,    -1,    -1,    66,    67,    -1,    69,
      -1,   174,   175,    73,    -1,    -1,    76,    -1,    -1,    -1,
      -1,    -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    -1,    -1,    -1,    94,    -1,    96,    97,    98,    -1,
     203,    -1,    -1,   206,    -1,   105,    -1,     4,     5,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    -1,    34,    -1,   139,
      -1,    -1,    -1,    40,   144,   145,   146,    44,    -1,   149,
      -1,   151,    49,    -1,    -1,   155,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,   164,    -1,   166,    -1,    -1,    66,
      67,    -1,    69,    -1,   174,   175,    73,    -1,    -1,    76,
      -1,    -1,    -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    -1,    96,
      97,    98,    -1,   203,    -1,    -1,   206,    -1,   105,    -1,
       4,     5,    -1,    -1,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    -1,    -1,    -1,
     127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,
      34,    -1,   139,    -1,    -1,    -1,    40,   144,   145,   146,
      44,    -1,   149,    -1,   151,    49,    -1,    -1,   155,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,   164,    -1,   166,
      -1,    -1,    66,    67,    -1,    69,    -1,   174,   175,    73,
      -1,    -1,    76,    -1,    -1,     5,    -1,    -1,    -1,    83,
      -1,    -1,    -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      94,    -1,    96,    97,    98,    -1,   203,    -1,    -1,   206,
      -1,   105,    32,    -1,    34,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    -1,    -1,    -1,    -1,   139,    66,    67,    -1,    69,
     144,   145,   146,    73,    -1,   149,    76,   151,    -1,    -1,
      -1,   155,    -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,
     164,    -1,   166,    -1,    94,    -1,    96,    97,    98,    -1,
     174,   175,    -1,    -1,    -1,   105,    -1,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
      -1,    -1,     4,     5,    -1,    -1,   202,   203,    -1,   203,
      -1,    13,   206,   209,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,    -1,    -1,   149,
      32,   151,    34,    -1,    -1,   155,    -1,    -1,    40,    41,
      -1,    -1,    44,    -1,   164,    -1,   166,    49,    -1,    -1,
      -1,    -1,    -1,    -1,   174,   175,    58,    -1,    60,    -1,
      -1,    -1,    -1,    -1,    66,    67,    -1,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    -1,   203,    -1,    -1,   206,    -1,    90,    91,
      -1,    -1,    94,    -1,    96,    97,    98,    99,    -1,    -1,
      -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,   110,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       4,     5,    -1,    -1,    -1,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,    -1,    -1,   149,    32,   151,
      34,    -1,    -1,   155,    -1,    -1,    40,    41,    -1,    -1,
      44,    -1,   164,    -1,   166,    49,    -1,    -1,    -1,    -1,
      -1,    -1,   174,   175,    58,    -1,    60,    -1,    -1,    -1,
      -1,    -1,    66,    67,    -1,    69,    -1,    -1,    -1,    73,
      -1,    -1,    76,    -1,    -1,    -1,    -1,    -1,    -1,    83,
      -1,   203,    -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      94,    -1,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
      -1,   105,    -1,    -1,    -1,    -1,   110,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,    -1,    -1,    -1,    -1,    -1,    -1,
       5,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,    -1,    -1,   149,    -1,   151,    -1,    -1,
      -1,   155,    -1,    -1,    -1,    -1,    -1,    32,   162,    34,
     164,    -1,   166,    -1,    39,    40,    -1,    -1,    -1,    44,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,
      -1,    66,    67,    -1,    69,    -1,    71,    -1,    73,   203,
      -1,    76,    -1,    -1,    -1,    -1,    -1,    -1,    83,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      -1,    96,    97,    98,    -1,    -1,    -1,    -1,    -1,    -1,
     105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     5,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,    -1,    -1,   149,    -1,   151,   152,    -1,    -1,
     155,    -1,    -1,    -1,    -1,    32,    -1,    34,    -1,   164,
      -1,   166,    39,    40,    -1,    -1,    -1,    44,    -1,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    -1,   192,    -1,    66,
      67,    -1,    69,    -1,    71,    -1,    73,    -1,   203,    76,
      -1,    -1,    -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    -1,    96,
      97,    98,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,     5,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    -1,    -1,    -1,    -1,   125,    23,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,
      34,    -1,   139,    -1,    -1,    -1,    40,   144,   145,   146,
      44,    -1,   149,    -1,   151,   152,    -1,    -1,   155,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,   164,    -1,   166,
      -1,    -1,    66,    67,    -1,    69,    -1,   174,   175,    73,
      -1,    -1,    76,    -1,    -1,     5,    -1,    -1,    -1,    83,
      -1,    -1,    -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,
      94,    -1,    96,    97,    98,    -1,   203,    -1,    -1,    -1,
      -1,   105,    32,    -1,    34,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    44,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    -1,    -1,    -1,    -1,   139,    66,    67,    -1,    69,
     144,   145,   146,    73,    -1,   149,    76,   151,    -1,    -1,
      -1,   155,    -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,
     164,    -1,   166,    -1,    94,    -1,    96,    97,    98,    -1,
     174,   175,    -1,    -1,   183,   105,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   202,   203,    -1,    -1,    -1,    -1,   203,
     209,    -1,    14,    15,    16,    17,    18,    19,    20,   139,
      22,    -1,    -1,    -1,   144,   145,   146,    -1,    -1,   149,
      -1,   151,    -1,    -1,    -1,   155,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   164,    -1,   166,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,   175,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   203,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    -1,   183,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   202,   203,    -1,    -1,    -1,    -1,    -1,
     209,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    14,    15,
      16,    17,    18,    19,    20,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     202,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   202,   203,   204,    -1,    -1,    -1,
      -1,    -1,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   202,   203,   204,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,
     203,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,     5,    -1,    -1,    -1,    -1,    -1,
     202,   203,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   188,   189,   190,   191,   192,   193,   194,   202,
     203,    32,    -1,    34,    -1,    -1,   202,   203,    -1,    40,
      41,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    -1,    60,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    69,    -1,
      -1,    -1,    73,    -1,    -1,    76,    -1,    -1,     5,    -1,
      -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    -1,    96,    97,    98,    99,    -1,
      -1,   102,    -1,    -1,   105,    32,    -1,    34,   109,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,   139,    66,
      67,    -1,    69,   144,   145,   146,    73,    -1,   149,    76,
     151,    -1,     5,    -1,   155,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    -1,   164,    -1,   166,    -1,    94,    -1,    96,
      97,    98,    -1,   174,   175,    -1,    -1,    -1,   105,    32,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,
      -1,    -1,   139,    66,    67,    -1,    -1,   144,   145,   146,
      73,    -1,   149,    76,   151,    -1,    -1,    -1,   155,    -1,
      83,    -1,    -1,    -1,    -1,    -1,    -1,   164,    -1,   166,
      -1,    -1,    -1,    96,    97,    98,    -1,   174,   175,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   144,   145,   146,    -1,    -1,   149,    -1,    -1,    -1,
      -1,    -1,   155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   164,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,   175,    24,    -1,    26,    27,    28,    29,    30,
      31,    -1,    33,    -1,    -1,    36,    -1,    38,    -1,    -1,
      -1,    42,    43,    -1,    45,    46,    47,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    -1,    59,    -1,
      61,    62,    63,    64,    65,    -1,    -1,    68,    -1,    -1,
      -1,    72,    -1,    74,    75,    -1,    77,    78,    79,    80,
      81,    82,    -1,    84,    85,    86,    87,    88,    89,    -1,
      -1,    92,    -1,    -1,    95,    -1,    -1,    -1,    -1,   100,
     101,    -1,   103,   104,    -1,   106,   107,    -1,    -1,    -1,
     111,   112,   113,   114,   115,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,    -1,   128,    -1,   130,
     131,   132,   133,   134,   135,   136,   137,   138,    -1,   140,
     141,   142,   143,    -1,    -1,    -1,   147,    -1,    -1,   150,
      -1,    -1,   153,   154,    -1,    -1,   157,   158,   159,   160,
      -1,    -1,   163,    -1,   165,    -1,   167,   168,   169,   170,
     171,   172,   173,    -1,    -1,   176
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   212,   213,     0,     4,     5,    32,    34,    40,    41,
      44,    49,    58,    60,    66,    67,    69,    73,    76,    83,
      90,    94,    96,    97,    98,    99,   105,   110,   127,   139,
     144,   145,   146,   149,   151,   155,   164,   166,   174,   175,
     203,   219,   220,   222,   223,   224,   225,   228,   229,   235,
     236,   247,   261,   265,   267,   268,   269,   270,   271,   274,
     275,   278,   280,   281,   282,   283,   285,   286,   287,   288,
     289,   291,   293,   311,   312,   313,   314,     3,     5,   208,
       3,     5,     3,     5,   263,    96,   266,     9,     3,     5,
     266,     3,   208,   208,   266,   267,     3,   263,     3,   263,
     267,    24,    26,    27,    28,    29,    30,    31,    33,    36,
      38,    42,    43,    45,    46,    47,    48,    50,    51,    52,
      53,    54,    55,    56,    57,    59,    61,    62,    63,    64,
      65,    68,    72,    74,    75,    77,    78,    79,    80,    81,
      82,    84,    85,    86,    87,    88,    89,    92,    95,   100,
     101,   103,   104,   106,   107,   111,   112,   113,   114,   115,
     116,   118,   119,   120,   121,   122,   123,   124,   126,   128,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   140,
     141,   142,   143,   147,   150,   153,   154,   157,   158,   159,
     160,   163,   165,   167,   168,   169,   170,   171,   172,   173,
     176,   237,   239,   310,   207,   215,   215,   162,    66,   102,
     109,   155,   164,   227,   247,   268,   274,   280,   284,   291,
     311,   314,   207,   207,   205,   205,   207,   205,   207,   218,
     207,   184,   279,   205,   292,   293,   292,   292,     3,     5,
      39,    71,   125,   152,   192,   208,   241,   264,   294,   295,
     308,   247,   311,   312,   314,   292,   207,     9,   205,   207,
       3,   312,   205,   156,   205,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   208,   208,   182,   204,
      13,    91,   219,   226,   229,   268,   270,   271,   282,   283,
     286,   206,   235,   236,     3,     3,     5,     3,     5,     3,
       3,   205,   308,   272,   214,   129,   276,   280,     3,     5,
     205,   217,   290,   294,   294,   210,   203,   208,   234,   292,
     209,   244,   245,   246,   264,   184,   209,   254,   208,   257,
       9,     9,    12,   240,   264,     3,     6,     7,     8,     9,
      10,    11,    70,   117,   148,   161,   187,   190,   191,   192,
     195,   196,   208,   250,   251,   252,   250,   253,     9,     9,
     238,   253,   252,     9,     9,   252,     9,   252,   250,   233,
     236,   291,   252,   248,   249,   250,     9,   310,     9,   252,
     312,   248,   250,   312,   177,   178,   179,   180,   181,   309,
     312,   312,   240,     6,     7,   315,     9,   312,   239,   203,
     208,   207,   205,   218,   291,   206,   235,   273,   213,   184,
     108,   235,   259,   277,   207,   205,   217,   206,   219,   236,
     289,   294,   209,   253,   192,   204,   250,   230,   231,   232,
     233,   206,   182,   210,     6,   221,   206,   235,   255,   259,
     206,   236,   256,   259,   209,   209,   209,   208,   250,   250,
     250,   250,   250,   250,   250,   291,    14,    15,    16,    17,
      18,    19,    20,    21,    22,   183,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   202,   203,   182,   209,
     209,   209,   182,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   291,   192,   208,   234,   241,   264,   300,
     302,   303,   209,   182,   209,   209,   209,   209,   182,   209,
     209,   209,   209,   209,   209,   209,   209,   202,   209,   209,
     209,   239,     9,   216,   294,   304,   218,   283,   206,   184,
     291,   311,   207,   206,   236,   260,   261,   206,   225,   206,
     218,   204,   204,   209,   182,   246,   252,     6,   209,   291,
     314,   209,   207,   207,   291,   209,   192,   208,   234,   241,
     296,   298,   299,   250,   250,     3,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,     3,   250,   252,     9,   302,   290,   192,
     230,   241,   301,   302,   208,   234,   249,   252,     6,   209,
     215,   182,   294,   207,   217,   218,    23,   233,   300,   306,
     307,   207,   262,   264,   298,   290,   192,   230,   241,   297,
     298,   209,   208,   234,   184,   204,   302,   290,   209,   302,
     209,   230,   209,   218,   206,   294,   206,   184,   305,   182,
     207,   205,   209,   298,   290,   209,   302,   209,   250,   230,
     250,   302,   209,   218,   253,   306,   242,   302,   209,    38,
      51,   206,   243,   252,   184,   184,   207,   258,   259,   258,
     207
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   211,   212,   213,   214,   213,   213,   213,   213,   213,
     213,   213,   213,   215,   215,   216,   215,   215,   215,   215,
     215,   215,   215,   215,   217,   217,   218,   218,   219,   219,
     219,   219,   219,   219,   219,   220,   221,   221,   222,   222,
     222,   222,   222,   222,   222,   222,   222,   223,   224,   225,
     226,   227,   227,   228,   229,   230,   230,   231,   231,   232,
     232,   233,   233,   234,   234,   234,   235,   235,   236,   237,
     237,   237,   238,   238,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   240,   240,   241,   241,   241,   241,
     242,   242,   243,   243,   244,   244,   244,   245,   245,   246,
     246,   247,   248,   248,   249,   249,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   251,   251,   252,
     253,   254,   254,   255,   255,   256,   256,   257,   257,   258,
     258,   259,   259,   260,   261,   261,   262,   262,   263,   263,
     263,   264,   264,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   266,   266,   267,   267,   267,   267,
     267,   267,   267,   267,   268,   268,   269,   270,   271,   272,
     272,   273,   274,   274,   275,   276,   276,   277,   277,   278,
     278,   279,   279,   280,   280,   281,   282,   282,   282,   283,
     283,   284,   284,   285,   286,   287,   287,   287,   288,   289,
     290,   290,   291,   291,   292,   292,   293,   293,   293,   294,
     294,   294,   295,   295,   295,   295,   296,   296,   296,   297,
     297,   298,   298,   299,   299,   299,   299,   299,   300,   300,
     300,   301,   301,   302,   302,   303,   303,   303,   303,   303,
     303,   304,   304,   305,   305,   306,   307,   307,   308,   308,
     309,   309,   309,   309,   309,   310,   310,   310,   311,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   313,
     314,   314,   315,   315,   315
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     0,     6,     2,     2,     3,     2,
       2,     2,     2,     0,     2,     0,     6,     2,     3,     2,
       2,     2,     2,     2,     0,     2,     0,     1,     1,     2,
       2,     1,     2,     1,     1,     6,     1,     2,     1,     2,
       1,     2,     1,     2,     2,     2,     2,     4,     3,     3,
       5,     2,     2,     3,     4,     0,     1,     1,     3,     1,
       3,     3,     2,     3,     3,     2,     0,     1,     3,     1,
       3,     4,     1,     3,     0,     1,     4,     1,     1,     1,
       1,     1,     4,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     1,     1,     1,     4,     1,
       1,     1,     4,     4,     1,     1,     1,     1,     4,     4,
       4,     4,     4,     1,     4,     1,     1,     4,     1,     4,
       1,     1,     4,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     1,
       1,     1,     4,     4,     1,     1,     1,     1,     1,     6,
       1,     4,     1,     1,     1,     4,     1,     1,     1,     4,
       4,     4,     4,     1,     1,     4,     4,     4,     1,     1,
       4,     4,     4,     1,     1,     1,     1,     1,     1,     1,
       0,     2,     4,     3,     0,     2,     1,     1,     3,     3,
       1,     5,     1,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     5,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     2,
       2,     3,     3,     5,     5,     4,     3,     1,     3,     1,
       1,     0,     2,     4,     3,     2,     2,     0,     2,     2,
       1,     3,     2,     1,     3,     2,     0,     1,     0,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     1,     1,
       1,     1,     1,     1,     0,     1,     1,     2,     1,     2,
       2,     1,     1,     1,     2,     2,     2,     5,     2,     0,
       2,     2,     2,     2,     2,     2,     3,     2,     3,     5,
       5,     0,     2,     2,     2,     2,     6,     8,     2,     2,
       2,     2,     2,     2,     5,     1,     1,     1,     1,     1,
       0,     2,     2,     3,     0,     1,     2,     2,     2,     3,
       2,     1,     1,     3,     2,     4,     3,     2,     1,     3,
       2,     0,     1,     3,     2,     1,     3,     4,     3,     2,
       1,     3,     2,     0,     1,     1,     3,     2,     1,     3,
       4,     1,     3,     0,     2,     2,     1,     3,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     5,     1,
       1,     1,     1,     2,     1,     2,     1,     2,     4,     5,
       5,    10,     1,     3,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 321 "parser.y" /* yacc.c:1646  */
    { fix_incomplete();
						  check_statements((yyvsp[0].stmt_list), FALSE);
						  check_all_user_types((yyvsp[0].stmt_list));
						  write_header((yyvsp[0].stmt_list));
						  write_id_data((yyvsp[0].stmt_list));
						  write_proxies((yyvsp[0].stmt_list));
						  write_client((yyvsp[0].stmt_list));
						  write_server((yyvsp[0].stmt_list));
						  write_regscript((yyvsp[0].stmt_list));
						  write_dlldata((yyvsp[0].stmt_list));
						  write_local_stubs((yyvsp[0].stmt_list));
						}
#line 2599 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 335 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = NULL; }
#line 2605 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 336 "parser.y" /* yacc.c:1646  */
    { push_namespace((yyvsp[-1].str)); }
#line 2611 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 337 "parser.y" /* yacc.c:1646  */
    { pop_namespace((yyvsp[-4].str)); (yyval.stmt_list) = append_statements((yyvsp[-5].stmt_list), (yyvsp[-1].stmt_list)); }
#line 2617 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 338 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_reference((yyvsp[0].type))); }
#line 2623 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 339 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_type_decl((yyvsp[0].type))); }
#line 2629 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 340 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = (yyvsp[-2].stmt_list);
						  reg_type((yyvsp[-1].type), (yyvsp[-1].type)->name, current_namespace, 0);
						}
#line 2637 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 343 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_type_decl((yyvsp[0].type)));
						  reg_type((yyvsp[0].type), (yyvsp[0].type)->name, current_namespace, 0);
						}
#line 2645 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 346 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_module((yyvsp[0].type))); }
#line 2651 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 347 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_library((yyvsp[0].typelib))); }
#line 2657 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 348 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), (yyvsp[0].statement)); }
#line 2663 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 351 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = NULL; }
#line 2669 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 352 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_reference((yyvsp[0].type))); }
#line 2675 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 353 "parser.y" /* yacc.c:1646  */
    { push_namespace((yyvsp[-1].str)); }
#line 2681 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 354 "parser.y" /* yacc.c:1646  */
    { pop_namespace((yyvsp[-4].str)); (yyval.stmt_list) = append_statements((yyvsp[-5].stmt_list), (yyvsp[-1].stmt_list)); }
#line 2687 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 355 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_type_decl((yyvsp[0].type))); }
#line 2693 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 356 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = (yyvsp[-2].stmt_list); reg_type((yyvsp[-1].type), (yyvsp[-1].type)->name, current_namespace, 0); }
#line 2699 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 357 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_type_decl((yyvsp[0].type)));
						  reg_type((yyvsp[0].type), (yyvsp[0].type)->name, current_namespace, 0);
						}
#line 2707 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 360 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_module((yyvsp[0].type))); }
#line 2713 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 361 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), (yyvsp[0].statement)); }
#line 2719 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 362 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_importlib((yyvsp[0].str))); }
#line 2725 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 363 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), make_statement_library((yyvsp[0].typelib))); }
#line 2731 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 366 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = NULL; }
#line 2737 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 367 "parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = append_statement((yyvsp[-1].stmt_list), (yyvsp[0].statement)); }
#line 2743 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 375 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = make_statement_cppquote((yyvsp[0].str)); }
#line 2749 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 376 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = make_statement_type_decl((yyvsp[-1].type)); }
#line 2755 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 377 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = make_statement_declaration((yyvsp[-1].var)); }
#line 2761 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 378 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = make_statement_import((yyvsp[0].str)); }
#line 2767 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 379 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[-1].statement); }
#line 2773 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 380 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = make_statement_pragma((yyvsp[0].str)); }
#line 2779 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 381 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = NULL; }
#line 2785 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 385 "parser.y" /* yacc.c:1646  */
    {
                      int result;
                      (yyval.statement) = NULL;
                      result = do_warning((yyvsp[-3].str), (yyvsp[-1].warning_list));
                      if(!result)
                          error_loc("expected \"disable\" or \"enable\"\n");
                  }
#line 2797 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 395 "parser.y" /* yacc.c:1646  */
    { (yyval.warning_list) = append_warning(NULL, (yyvsp[0].num)); }
#line 2803 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 396 "parser.y" /* yacc.c:1646  */
    { (yyval.warning_list) = append_warning((yyvsp[-1].warning_list), (yyvsp[0].num)); }
#line 2809 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 401 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_enum((yyvsp[0].str), current_namespace, FALSE, NULL); }
#line 2815 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 403 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_struct((yyvsp[0].str), current_namespace, FALSE, NULL); }
#line 2821 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 405 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_nonencapsulated_union((yyvsp[0].str), FALSE, NULL); }
#line 2827 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 406 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[0].type); (yyval.type)->attrs = check_enum_attrs((yyvsp[-1].attr_list)); }
#line 2833 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 407 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[0].type); (yyval.type)->attrs = check_struct_attrs((yyvsp[-1].attr_list)); }
#line 2839 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 408 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[0].type); (yyval.type)->attrs = check_union_attrs((yyvsp[-1].attr_list)); }
#line 2845 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 411 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = (yyvsp[-1].str); }
#line 2851 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 413 "parser.y" /* yacc.c:1646  */
    { assert(yychar == YYEMPTY);
						  (yyval.import) = xmalloc(sizeof(struct _import_t));
						  (yyval.import)->name = (yyvsp[-1].str);
						  (yyval.import)->import_performed = do_import((yyvsp[-1].str));
						  if (!(yyval.import)->import_performed) yychar = aEOF;
						}
#line 2862 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 421 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = (yyvsp[-2].import)->name;
						  if ((yyvsp[-2].import)->import_performed) pop_import();
						  free((yyvsp[-2].import));
						}
#line 2871 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 428 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = (yyvsp[-2].str); if(!parse_only) add_importlib((yyvsp[-2].str)); }
#line 2877 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 431 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = (yyvsp[0].str); }
#line 2883 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 432 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = (yyvsp[0].str); }
#line 2889 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 434 "parser.y" /* yacc.c:1646  */
    { (yyval.typelib) = make_library((yyvsp[-1].str), check_library_attrs((yyvsp[-1].str), (yyvsp[-2].attr_list)));
						  if (!parse_only) start_typelib((yyval.typelib));
						}
#line 2897 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 439 "parser.y" /* yacc.c:1646  */
    { (yyval.typelib) = (yyvsp[-3].typelib);
						  (yyval.typelib)->stmts = (yyvsp[-2].stmt_list);
						  if (!parse_only) end_typelib();
						}
#line 2906 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 445 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = NULL; }
#line 2912 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 449 "parser.y" /* yacc.c:1646  */
    { check_arg_attrs((yyvsp[0].var)); (yyval.var_list) = append_var( NULL, (yyvsp[0].var) ); }
#line 2918 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 450 "parser.y" /* yacc.c:1646  */
    { check_arg_attrs((yyvsp[0].var)); (yyval.var_list) = append_var( (yyvsp[-2].var_list), (yyvsp[0].var) ); }
#line 2924 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 454 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = append_var( (yyvsp[-2].var_list), make_var(strdup("...")) ); }
#line 2930 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 458 "parser.y" /* yacc.c:1646  */
    { if ((yyvsp[-1].declspec)->stgclass != STG_NONE && (yyvsp[-1].declspec)->stgclass != STG_REGISTER)
						    error_loc("invalid storage class for function parameter\n");
						  (yyval.var) = declare_var((yyvsp[-2].attr_list), (yyvsp[-1].declspec), (yyvsp[0].declarator), TRUE);
						  free((yyvsp[-1].declspec)); free((yyvsp[0].declarator));
						}
#line 2940 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 463 "parser.y" /* yacc.c:1646  */
    { if ((yyvsp[-1].declspec)->stgclass != STG_NONE && (yyvsp[-1].declspec)->stgclass != STG_REGISTER)
						    error_loc("invalid storage class for function parameter\n");
						  (yyval.var) = declare_var(NULL, (yyvsp[-1].declspec), (yyvsp[0].declarator), TRUE);
						  free((yyvsp[-1].declspec)); free((yyvsp[0].declarator));
						}
#line 2950 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 470 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr);
						  if (!(yyval.expr)->is_const)
						      error_loc("array dimension is not an integer constant\n");
						}
#line 2959 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 474 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr(EXPR_VOID); }
#line 2965 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 475 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr(EXPR_VOID); }
#line 2971 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 478 "parser.y" /* yacc.c:1646  */
    { (yyval.attr_list) = NULL; }
#line 2977 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 483 "parser.y" /* yacc.c:1646  */
    { (yyval.attr_list) = (yyvsp[-1].attr_list); }
#line 2983 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 486 "parser.y" /* yacc.c:1646  */
    { (yyval.attr_list) = append_attr( NULL, (yyvsp[0].attr) ); }
#line 2989 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 487 "parser.y" /* yacc.c:1646  */
    { (yyval.attr_list) = append_attr( (yyvsp[-2].attr_list), (yyvsp[0].attr) ); }
#line 2995 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 488 "parser.y" /* yacc.c:1646  */
    { (yyval.attr_list) = append_attr( (yyvsp[-3].attr_list), (yyvsp[0].attr) ); }
#line 3001 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 491 "parser.y" /* yacc.c:1646  */
    { (yyval.str_list) = append_str( NULL, (yyvsp[0].str) ); }
#line 3007 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 492 "parser.y" /* yacc.c:1646  */
    { (yyval.str_list) = append_str( (yyvsp[-2].str_list), (yyvsp[0].str) ); }
#line 3013 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 495 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = NULL; }
#line 3019 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 496 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_AGGREGATABLE); }
#line 3025 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 497 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_ANNOTATION, (yyvsp[-1].str)); }
#line 3031 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 498 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_APPOBJECT); }
#line 3037 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 499 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_ASYNC); }
#line 3043 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 500 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_AUTO_HANDLE); }
#line 3049 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 501 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_BINDABLE); }
#line 3055 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 502 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_BROADCAST); }
#line 3061 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 503 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_CALLAS, (yyvsp[-1].var)); }
#line 3067 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 504 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_CASE, (yyvsp[-1].expr_list)); }
#line 3073 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 505 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_CODE); }
#line 3079 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 506 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_COMMSTATUS); }
#line 3085 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 507 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrv(ATTR_CONTEXTHANDLE, 0); }
#line 3091 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 508 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrv(ATTR_CONTEXTHANDLE, 0); /* RPC_CONTEXT_HANDLE_DONT_SERIALIZE */ }
#line 3097 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 509 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrv(ATTR_CONTEXTHANDLE, 0); /* RPC_CONTEXT_HANDLE_SERIALIZE */ }
#line 3103 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 510 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_CONTROL); }
#line 3109 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 511 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_DECODE); }
#line 3115 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 512 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_DEFAULT); }
#line 3121 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 513 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_DEFAULTBIND); }
#line 3127 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 514 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_DEFAULTCOLLELEM); }
#line 3133 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 515 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_DEFAULTVALUE, (yyvsp[-1].expr)); }
#line 3139 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 516 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_DEFAULTVTABLE); }
#line 3145 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 517 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_DISABLECONSISTENCYCHECK); }
#line 3151 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 518 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_DISPLAYBIND); }
#line 3157 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 519 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_DLLNAME, (yyvsp[-1].str)); }
#line 3163 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 520 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_DUAL); }
#line 3169 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 521 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_ENABLEALLOCATE); }
#line 3175 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 522 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_ENCODE); }
#line 3181 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 523 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_ENDPOINT, (yyvsp[-1].str_list)); }
#line 3187 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 524 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_ENTRY, (yyvsp[-1].expr)); }
#line 3193 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 525 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_EXPLICIT_HANDLE); }
#line 3199 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 526 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_FAULTSTATUS); }
#line 3205 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 527 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_FORCEALLOCATE); }
#line 3211 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 528 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_HANDLE); }
#line 3217 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 529 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_HELPCONTEXT, (yyvsp[-1].expr)); }
#line 3223 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 530 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_HELPFILE, (yyvsp[-1].str)); }
#line 3229 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 531 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_HELPSTRING, (yyvsp[-1].str)); }
#line 3235 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 532 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_HELPSTRINGCONTEXT, (yyvsp[-1].expr)); }
#line 3241 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 533 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_HELPSTRINGDLL, (yyvsp[-1].str)); }
#line 3247 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 534 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_HIDDEN); }
#line 3253 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 535 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_ID, (yyvsp[-1].expr)); }
#line 3259 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 536 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_IDEMPOTENT); }
#line 3265 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 537 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_IGNORE); }
#line 3271 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 538 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_IIDIS, (yyvsp[-1].expr)); }
#line 3277 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 539 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_IMMEDIATEBIND); }
#line 3283 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 540 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_IMPLICIT_HANDLE, (yyvsp[-1].var)); }
#line 3289 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 541 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_IN); }
#line 3295 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 542 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_INPUTSYNC); }
#line 3301 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 543 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_LENGTHIS, (yyvsp[-1].expr_list)); }
#line 3307 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 544 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_LIBLCID, (yyvsp[-1].expr)); }
#line 3313 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 545 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_PARAMLCID); }
#line 3319 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 546 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_LICENSED); }
#line 3325 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 547 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_LOCAL); }
#line 3331 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 548 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_MAYBE); }
#line 3337 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 549 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_MESSAGE); }
#line 3343 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 550 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_NOCODE); }
#line 3349 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 551 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_NONBROWSABLE); }
#line 3355 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 552 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_NONCREATABLE); }
#line 3361 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 553 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_NONEXTENSIBLE); }
#line 3367 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 554 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_NOTIFY); }
#line 3373 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 555 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_NOTIFYFLAG); }
#line 3379 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 556 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_OBJECT); }
#line 3385 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 557 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_ODL); }
#line 3391 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 558 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_OLEAUTOMATION); }
#line 3397 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 559 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_OPTIMIZE, (yyvsp[-1].str)); }
#line 3403 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 560 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_OPTIONAL); }
#line 3409 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 561 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_OUT); }
#line 3415 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 562 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_PARTIALIGNORE); }
#line 3421 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 563 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrv(ATTR_POINTERDEFAULT, (yyvsp[-1].num)); }
#line 3427 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 564 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_PROGID, (yyvsp[-1].str)); }
#line 3433 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 565 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_PROPGET); }
#line 3439 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 566 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_PROPPUT); }
#line 3445 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 567 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_PROPPUTREF); }
#line 3451 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 568 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_PROXY); }
#line 3457 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 569 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_PUBLIC); }
#line 3463 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 571 "parser.y" /* yacc.c:1646  */
    { expr_list_t *list = append_expr( NULL, (yyvsp[-3].expr) );
						  list = append_expr( list, (yyvsp[-1].expr) );
						  (yyval.attr) = make_attrp(ATTR_RANGE, list); }
#line 3471 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 574 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_READONLY); }
#line 3477 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 575 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_REPRESENTAS, (yyvsp[-1].type)); }
#line 3483 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 576 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_REQUESTEDIT); }
#line 3489 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 577 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_RESTRICTED); }
#line 3495 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 578 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_RETVAL); }
#line 3501 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 579 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_SIZEIS, (yyvsp[-1].expr_list)); }
#line 3507 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 580 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_SOURCE); }
#line 3513 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 581 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_STRICTCONTEXTHANDLE); }
#line 3519 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 582 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_STRING); }
#line 3525 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 583 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_SWITCHIS, (yyvsp[-1].expr)); }
#line 3531 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 584 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_SWITCHTYPE, (yyvsp[-1].type)); }
#line 3537 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 585 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_TRANSMITAS, (yyvsp[-1].type)); }
#line 3543 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 586 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrv(ATTR_THREADING, (yyvsp[-1].num)); }
#line 3549 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 587 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_UIDEFAULT); }
#line 3555 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 588 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_USESGETLASTERROR); }
#line 3561 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 589 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_USERMARSHAL, (yyvsp[-1].type)); }
#line 3567 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 590 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_UUID, (yyvsp[-1].uuid)); }
#line 3573 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 591 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_ASYNCUUID, (yyvsp[-1].uuid)); }
#line 3579 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 592 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_V1ENUM); }
#line 3585 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 593 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_VARARG); }
#line 3591 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 594 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrv(ATTR_VERSION, (yyvsp[-1].num)); }
#line 3597 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 595 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_VIPROGID, (yyvsp[-1].str)); }
#line 3603 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 596 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrp(ATTR_WIREMARSHAL, (yyvsp[-1].type)); }
#line 3609 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 597 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attrv(ATTR_POINTERTYPE, (yyvsp[0].num)); }
#line 3615 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 602 "parser.y" /* yacc.c:1646  */
    { if (!is_valid_uuid((yyvsp[0].str)))
						    error_loc("invalid UUID: %s\n", (yyvsp[0].str));
						  (yyval.uuid) = parse_uuid((yyvsp[0].str)); }
#line 3623 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 607 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = xstrdup("__cdecl"); }
#line 3629 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 608 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = xstrdup("__fastcall"); }
#line 3635 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 609 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = xstrdup("__pascal"); }
#line 3641 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 610 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = xstrdup("__stdcall"); }
#line 3647 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 613 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = NULL; }
#line 3653 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 614 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = append_var( (yyvsp[-1].var_list), (yyvsp[0].var) ); }
#line 3659 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 617 "parser.y" /* yacc.c:1646  */
    { attr_t *a = make_attrp(ATTR_CASE, append_expr( NULL, (yyvsp[-2].expr) ));
						  (yyval.var) = (yyvsp[0].var); if (!(yyval.var)) (yyval.var) = make_var(NULL);
						  (yyval.var)->attrs = append_attr( (yyval.var)->attrs, a );
						}
#line 3668 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 621 "parser.y" /* yacc.c:1646  */
    { attr_t *a = make_attr(ATTR_DEFAULT);
						  (yyval.var) = (yyvsp[0].var); if (!(yyval.var)) (yyval.var) = make_var(NULL);
						  (yyval.var)->attrs = append_attr( (yyval.var)->attrs, a );
						}
#line 3677 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 627 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = NULL; }
#line 3683 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 628 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = (yyvsp[-1].var_list); }
#line 3689 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 632 "parser.y" /* yacc.c:1646  */
    { if (!(yyvsp[0].var)->eval)
						    (yyvsp[0].var)->eval = make_exprl(EXPR_NUM, 0 /* default for first enum entry */);
                                                  (yyval.var_list) = append_var( NULL, (yyvsp[0].var) );
						}
#line 3698 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 636 "parser.y" /* yacc.c:1646  */
    { if (!(yyvsp[0].var)->eval)
                                                  {
                                                    var_t *last = LIST_ENTRY( list_tail((yyval.var_list)), var_t, entry );
                                                    enum expr_type type = EXPR_NUM;
                                                    if (last->eval->type == EXPR_HEXNUM) type = EXPR_HEXNUM;
                                                    if (last->eval->cval + 1 < 0) type = EXPR_HEXNUM;
                                                    (yyvsp[0].var)->eval = make_exprl(type, last->eval->cval + 1);
                                                  }
                                                  (yyval.var_list) = append_var( (yyvsp[-2].var_list), (yyvsp[0].var) );
						}
#line 3713 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 648 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = reg_const((yyvsp[-2].var));
						  (yyval.var)->eval = (yyvsp[0].expr);
                                                  (yyval.var)->type = type_new_int(TYPE_BASIC_INT, 0);
						}
#line 3722 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 652 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = reg_const((yyvsp[0].var));
                                                  (yyval.var)->type = type_new_int(TYPE_BASIC_INT, 0);
						}
#line 3730 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 657 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_enum((yyvsp[-3].str), current_namespace, TRUE, (yyvsp[-1].var_list)); }
#line 3736 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 660 "parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = append_expr( NULL, (yyvsp[0].expr) ); }
#line 3742 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 661 "parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = append_expr( (yyvsp[-2].expr_list), (yyvsp[0].expr) ); }
#line 3748 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 664 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr(EXPR_VOID); }
#line 3754 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 668 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprl(EXPR_NUM, (yyvsp[0].num)); }
#line 3760 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 669 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprl(EXPR_HEXNUM, (yyvsp[0].num)); }
#line 3766 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 670 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprd(EXPR_DOUBLE, (yyvsp[0].dbl)); }
#line 3772 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 671 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprl(EXPR_TRUEFALSE, 0); }
#line 3778 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 672 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprl(EXPR_NUM, 0); }
#line 3784 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 673 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprl(EXPR_TRUEFALSE, 1); }
#line 3790 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 674 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprs(EXPR_STRLIT, (yyvsp[0].str)); }
#line 3796 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 675 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprs(EXPR_WSTRLIT, (yyvsp[0].str)); }
#line 3802 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 676 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprs(EXPR_CHARCONST, (yyvsp[0].str)); }
#line 3808 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 677 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprs(EXPR_IDENTIFIER, (yyvsp[0].str)); }
#line 3814 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 678 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr3(EXPR_COND, (yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3820 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 679 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_LOGOR, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3826 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 680 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_LOGAND, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3832 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 681 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_OR , (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3838 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 682 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_XOR, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3844 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 683 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_AND, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3850 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 684 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_EQUALITY, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3856 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 685 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_INEQUALITY, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3862 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 686 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_GTR, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3868 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 687 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_LESS, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3874 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 688 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_GTREQL, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3880 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 689 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_LESSEQL, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3886 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 690 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_SHL, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3892 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 691 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_SHR, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3898 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 692 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_ADD, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3904 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 693 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_SUB, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3910 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 694 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_MOD, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3916 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 695 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_MUL, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3922 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 696 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_DIV, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3928 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 697 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr1(EXPR_LOGNOT, (yyvsp[0].expr)); }
#line 3934 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 698 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr1(EXPR_NOT, (yyvsp[0].expr)); }
#line 3940 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 699 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr1(EXPR_POS, (yyvsp[0].expr)); }
#line 3946 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 700 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr1(EXPR_NEG, (yyvsp[0].expr)); }
#line 3952 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 701 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr1(EXPR_ADDRESSOF, (yyvsp[0].expr)); }
#line 3958 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 702 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr1(EXPR_PPTR, (yyvsp[0].expr)); }
#line 3964 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 703 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_MEMBER, make_expr1(EXPR_PPTR, (yyvsp[-2].expr)), make_exprs(EXPR_IDENTIFIER, (yyvsp[0].str))); }
#line 3970 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 704 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_MEMBER, (yyvsp[-2].expr), make_exprs(EXPR_IDENTIFIER, (yyvsp[0].str))); }
#line 3976 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 706 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprt(EXPR_CAST, declare_var(NULL, (yyvsp[-3].declspec), (yyvsp[-2].declarator), 0), (yyvsp[0].expr)); free((yyvsp[-3].declspec)); free((yyvsp[-2].declarator)); }
#line 3982 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 708 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_exprt(EXPR_SIZEOF, declare_var(NULL, (yyvsp[-2].declspec), (yyvsp[-1].declarator), 0), NULL); free((yyvsp[-2].declspec)); free((yyvsp[-1].declarator)); }
#line 3988 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 709 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = make_expr2(EXPR_ARRAY, (yyvsp[-3].expr), (yyvsp[-1].expr)); }
#line 3994 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 710 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 4000 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 713 "parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = append_expr( NULL, (yyvsp[0].expr) ); }
#line 4006 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 714 "parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = append_expr( (yyvsp[-2].expr_list), (yyvsp[0].expr) ); }
#line 4012 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 717 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr);
						  if (!(yyval.expr)->is_const)
						      error_loc("expression is not an integer constant\n");
						}
#line 4021 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 723 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr);
						  if (!(yyval.expr)->is_const && (yyval.expr)->type != EXPR_STRLIT && (yyval.expr)->type != EXPR_WSTRLIT)
						      error_loc("expression is not constant\n");
						}
#line 4030 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 729 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = NULL; }
#line 4036 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 730 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = append_var_list((yyvsp[-1].var_list), (yyvsp[0].var_list)); }
#line 4042 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 734 "parser.y" /* yacc.c:1646  */
    { const char *first = LIST_ENTRY(list_head((yyvsp[-1].declarator_list)), declarator_t, entry)->var->name;
						  check_field_attrs(first, (yyvsp[-3].attr_list));
						  (yyval.var_list) = set_var_types((yyvsp[-3].attr_list), (yyvsp[-2].declspec), (yyvsp[-1].declarator_list));
						}
#line 4051 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 738 "parser.y" /* yacc.c:1646  */
    { var_t *v = make_var(NULL);
						  v->type = (yyvsp[-1].type); v->attrs = (yyvsp[-2].attr_list);
						  (yyval.var_list) = append_var(NULL, v);
						}
#line 4060 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 745 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 4066 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 746 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = make_var(NULL); (yyval.var)->attrs = (yyvsp[-1].attr_list); }
#line 4072 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 749 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = NULL; }
#line 4078 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 750 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = append_var( (yyvsp[-1].var_list), (yyvsp[0].var) ); }
#line 4084 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 754 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[-1].var); }
#line 4090 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 755 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = NULL; }
#line 4096 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 758 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = declare_var(check_field_attrs((yyvsp[0].declarator)->var->name, (yyvsp[-2].attr_list)),
						                (yyvsp[-1].declspec), (yyvsp[0].declarator), FALSE);
						  free((yyvsp[0].declarator));
						}
#line 4105 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 762 "parser.y" /* yacc.c:1646  */
    { var_t *v = make_var(NULL);
						  v->type = (yyvsp[0].type); v->attrs = (yyvsp[-1].attr_list);
						  (yyval.var) = v;
						}
#line 4114 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 253:
#line 768 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = (yyvsp[0].var);
						  if (type_get_type((yyval.var)->type) != TYPE_FUNCTION)
						    error_loc("only methods may be declared inside the methods section of a dispinterface\n");
						  check_function_attrs((yyval.var)->name, (yyval.var)->attrs);
						}
#line 4124 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 777 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = declare_var((yyvsp[-2].attr_list), (yyvsp[-1].declspec), (yyvsp[0].declarator), FALSE);
						  free((yyvsp[0].declarator));
						}
#line 4132 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 780 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = declare_var(NULL, (yyvsp[-1].declspec), (yyvsp[0].declarator), FALSE);
						  free((yyvsp[0].declarator));
						}
#line 4140 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 785 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = NULL; }
#line 4146 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 789 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = NULL; }
#line 4152 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 790 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = (yyvsp[0].str); }
#line 4158 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 791 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = (yyvsp[0].str); }
#line 4164 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 794 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = make_var((yyvsp[0].str)); }
#line 4170 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 796 "parser.y" /* yacc.c:1646  */
    { (yyval.var) = make_var((yyvsp[0].str)); }
#line 4176 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 799 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = find_type_or_error((yyvsp[0].str), 0); }
#line 4182 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 800 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = find_type_or_error((yyvsp[0].str), 0); }
#line 4188 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 802 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(type_basic_get_type((yyvsp[0].type)), -1); }
#line 4194 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 803 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(type_basic_get_type((yyvsp[0].type)), 1); }
#line 4200 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 804 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(TYPE_BASIC_INT, 1); }
#line 4206 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 805 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = find_type_or_error((yyvsp[0].str), 0); }
#line 4212 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 806 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = find_type_or_error((yyvsp[0].str), 0); }
#line 4218 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 807 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = find_type_or_error((yyvsp[0].str), 0); }
#line 4224 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 808 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = find_type_or_error((yyvsp[0].str), 0); }
#line 4230 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 809 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = find_type_or_error((yyvsp[0].str), 0); }
#line 4236 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 276:
#line 816 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(TYPE_BASIC_INT, 0); }
#line 4242 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 817 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(TYPE_BASIC_INT16, 0); }
#line 4248 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 818 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(TYPE_BASIC_INT8, 0); }
#line 4254 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 819 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(TYPE_BASIC_INT32, 0); }
#line 4260 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 820 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(TYPE_BASIC_HYPER, 0); }
#line 4266 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 821 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(TYPE_BASIC_INT64, 0); }
#line 4272 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 282:
#line 822 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(TYPE_BASIC_CHAR, 0); }
#line 4278 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 283:
#line 823 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_int(TYPE_BASIC_INT3264, 0); }
#line 4284 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 284:
#line 826 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_coclass((yyvsp[0].str)); }
#line 4290 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 285:
#line 827 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = find_type((yyvsp[0].str), NULL, 0);
						  if (type_get_type_detect_alias((yyval.type)) != TYPE_COCLASS)
						    error_loc("%s was not declared a coclass at %s:%d\n",
							      (yyvsp[0].str), (yyval.type)->loc_info.input_name,
							      (yyval.type)->loc_info.line_number);
						}
#line 4301 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 286:
#line 835 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[0].type);
						  check_def((yyval.type));
						  (yyval.type)->attrs = check_coclass_attrs((yyvsp[0].type)->name, (yyvsp[-1].attr_list));
						}
#line 4310 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 287:
#line 842 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_coclass_define((yyvsp[-4].type), (yyvsp[-2].ifref_list)); }
#line 4316 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 288:
#line 845 "parser.y" /* yacc.c:1646  */
    { (yyval.str) = (yyvsp[0].str); }
#line 4322 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 289:
#line 848 "parser.y" /* yacc.c:1646  */
    { (yyval.ifref_list) = NULL; }
#line 4328 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 290:
#line 849 "parser.y" /* yacc.c:1646  */
    { (yyval.ifref_list) = append_ifref( (yyvsp[-1].ifref_list), (yyvsp[0].ifref) ); }
#line 4334 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 291:
#line 853 "parser.y" /* yacc.c:1646  */
    { (yyval.ifref) = make_ifref((yyvsp[0].type)); (yyval.ifref)->attrs = (yyvsp[-1].attr_list); }
#line 4340 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 292:
#line 856 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = get_type(TYPE_INTERFACE, (yyvsp[0].str), current_namespace, 0); }
#line 4346 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 293:
#line 857 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = get_type(TYPE_INTERFACE, (yyvsp[0].str), current_namespace, 0); }
#line 4352 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 294:
#line 860 "parser.y" /* yacc.c:1646  */
    { attr_t *attrs;
						  (yyval.type) = (yyvsp[0].type);
						  check_def((yyval.type));
						  attrs = make_attr(ATTR_DISPINTERFACE);
						  (yyval.type)->attrs = append_attr( check_dispiface_attrs((yyvsp[0].type)->name, (yyvsp[-1].attr_list)), attrs );
						  (yyval.type)->defined = TRUE;
						}
#line 4364 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 295:
#line 869 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = NULL; }
#line 4370 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 296:
#line 870 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = append_var( (yyvsp[-2].var_list), (yyvsp[-1].var) ); }
#line 4376 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 297:
#line 873 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = NULL; }
#line 4382 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 298:
#line 874 "parser.y" /* yacc.c:1646  */
    { (yyval.var_list) = append_var( (yyvsp[-2].var_list), (yyvsp[-1].var) ); }
#line 4388 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 299:
#line 880 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-4].type);
						  type_dispinterface_define((yyval.type), (yyvsp[-2].var_list), (yyvsp[-1].var_list));
						}
#line 4396 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 300:
#line 884 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-4].type);
						  type_dispinterface_define_from_iface((yyval.type), (yyvsp[-2].type));
						}
#line 4404 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 301:
#line 889 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = NULL; }
#line 4410 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 302:
#line 890 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = find_type_or_error2((yyvsp[0].str), 0); }
#line 4416 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 303:
#line 893 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = get_type(TYPE_INTERFACE, (yyvsp[0].str), current_namespace, 0); }
#line 4422 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 304:
#line 894 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = get_type(TYPE_INTERFACE, (yyvsp[0].str), current_namespace, 0); }
#line 4428 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 305:
#line 897 "parser.y" /* yacc.c:1646  */
    { (yyval.ifinfo).interface = (yyvsp[0].type);
						  (yyval.ifinfo).old_pointer_default = pointer_default;
						  if (is_attr((yyvsp[-1].attr_list), ATTR_POINTERDEFAULT))
						    pointer_default = get_attrv((yyvsp[-1].attr_list), ATTR_POINTERDEFAULT);
						  check_def((yyvsp[0].type));
						  (yyvsp[0].type)->attrs = check_iface_attrs((yyvsp[0].type)->name, (yyvsp[-1].attr_list));
						  (yyvsp[0].type)->defined = TRUE;
						}
#line 4441 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 306:
#line 908 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-5].ifinfo).interface;
						  if((yyval.type) == (yyvsp[-4].type))
						    error_loc("Interface can't inherit from itself\n");
						  type_interface_define((yyval.type), (yyvsp[-4].type), (yyvsp[-2].stmt_list));
						  pointer_default = (yyvsp[-5].ifinfo).old_pointer_default;
						}
#line 4452 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 307:
#line 918 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-7].ifinfo).interface;
						  type_interface_define((yyval.type), find_type_or_error2((yyvsp[-5].str), 0), (yyvsp[-2].stmt_list));
						  pointer_default = (yyvsp[-7].ifinfo).old_pointer_default;
						}
#line 4461 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 308:
#line 922 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-1].type); }
#line 4467 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 309:
#line 926 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-1].type); }
#line 4473 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 310:
#line 927 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-1].type); }
#line 4479 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 311:
#line 930 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_module((yyvsp[0].str)); }
#line 4485 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 312:
#line 931 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_module((yyvsp[0].str)); }
#line 4491 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 313:
#line 934 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[0].type);
						  (yyval.type)->attrs = check_module_attrs((yyvsp[0].type)->name, (yyvsp[-1].attr_list));
						}
#line 4499 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 314:
#line 940 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[-4].type);
                                                  type_module_define((yyval.type), (yyvsp[-2].stmt_list));
						}
#line 4507 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 315:
#line 946 "parser.y" /* yacc.c:1646  */
    { (yyval.stgclass) = STG_EXTERN; }
#line 4513 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 316:
#line 947 "parser.y" /* yacc.c:1646  */
    { (yyval.stgclass) = STG_STATIC; }
#line 4519 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 317:
#line 948 "parser.y" /* yacc.c:1646  */
    { (yyval.stgclass) = STG_REGISTER; }
#line 4525 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 318:
#line 952 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_INLINE); }
#line 4531 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 319:
#line 956 "parser.y" /* yacc.c:1646  */
    { (yyval.attr) = make_attr(ATTR_CONST); }
#line 4537 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 320:
#line 959 "parser.y" /* yacc.c:1646  */
    { (yyval.attr_list) = NULL; }
#line 4543 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 321:
#line 960 "parser.y" /* yacc.c:1646  */
    { (yyval.attr_list) = append_attr((yyvsp[-1].attr_list), (yyvsp[0].attr)); }
#line 4549 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 322:
#line 963 "parser.y" /* yacc.c:1646  */
    { (yyval.declspec) = make_decl_spec((yyvsp[-1].type), (yyvsp[0].declspec), NULL, NULL, STG_NONE); }
#line 4555 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 323:
#line 965 "parser.y" /* yacc.c:1646  */
    { (yyval.declspec) = make_decl_spec((yyvsp[-1].type), (yyvsp[-2].declspec), (yyvsp[0].declspec), NULL, STG_NONE); }
#line 4561 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 324:
#line 968 "parser.y" /* yacc.c:1646  */
    { (yyval.declspec) = NULL; }
#line 4567 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 326:
#line 973 "parser.y" /* yacc.c:1646  */
    { (yyval.declspec) = make_decl_spec(NULL, (yyvsp[0].declspec), NULL, (yyvsp[-1].attr), STG_NONE); }
#line 4573 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 327:
#line 974 "parser.y" /* yacc.c:1646  */
    { (yyval.declspec) = make_decl_spec(NULL, (yyvsp[0].declspec), NULL, (yyvsp[-1].attr), STG_NONE); }
#line 4579 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 328:
#line 975 "parser.y" /* yacc.c:1646  */
    { (yyval.declspec) = make_decl_spec(NULL, (yyvsp[0].declspec), NULL, NULL, (yyvsp[-1].stgclass)); }
#line 4585 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 329:
#line 980 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); (yyval.declarator)->type = append_ptrchain_type((yyval.declarator)->type, type_new_pointer(pointer_default, NULL, (yyvsp[-1].attr_list))); }
#line 4591 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 330:
#line 981 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); if ((yyval.declarator)->func_type) (yyval.declarator)->func_type->attrs = append_attr((yyval.declarator)->func_type->attrs, make_attrp(ATTR_CALLCONV, (yyvsp[-1].str)));
						           else if ((yyval.declarator)->type) (yyval.declarator)->type->attrs = append_attr((yyval.declarator)->type->attrs, make_attrp(ATTR_CALLCONV, (yyvsp[-1].str))); }
#line 4598 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 332:
#line 987 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = make_declarator((yyvsp[0].var)); }
#line 4604 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 333:
#line 988 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-1].declarator); }
#line 4610 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 334:
#line 989 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-1].declarator); (yyval.declarator)->array = append_array((yyval.declarator)->array, (yyvsp[0].expr)); }
#line 4616 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 335:
#line 990 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-3].declarator);
						  (yyval.declarator)->func_type = append_ptrchain_type((yyval.declarator)->type, type_new_function((yyvsp[-1].var_list)));
						  (yyval.declarator)->type = NULL;
						}
#line 4625 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 336:
#line 999 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); (yyval.declarator)->type = append_ptrchain_type((yyval.declarator)->type, type_new_pointer(pointer_default, NULL, (yyvsp[-1].attr_list))); }
#line 4631 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 337:
#line 1000 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); if ((yyval.declarator)->func_type) (yyval.declarator)->func_type->attrs = append_attr((yyval.declarator)->func_type->attrs, make_attrp(ATTR_CALLCONV, (yyvsp[-1].str)));
						           else if ((yyval.declarator)->type) (yyval.declarator)->type->attrs = append_attr((yyval.declarator)->type->attrs, make_attrp(ATTR_CALLCONV, (yyvsp[-1].str))); }
#line 4638 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 339:
#line 1008 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); (yyval.declarator)->type = append_ptrchain_type((yyval.declarator)->type, type_new_pointer(pointer_default, NULL, (yyvsp[-1].attr_list))); }
#line 4644 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 340:
#line 1009 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); if ((yyval.declarator)->func_type) (yyval.declarator)->func_type->attrs = append_attr((yyval.declarator)->func_type->attrs, make_attrp(ATTR_CALLCONV, (yyvsp[-1].str)));
						           else if ((yyval.declarator)->type) (yyval.declarator)->type->attrs = append_attr((yyval.declarator)->type->attrs, make_attrp(ATTR_CALLCONV, (yyvsp[-1].str))); }
#line 4651 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 341:
#line 1014 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = make_declarator(NULL); }
#line 4657 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 343:
#line 1020 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-1].declarator); }
#line 4663 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 344:
#line 1021 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-1].declarator); (yyval.declarator)->array = append_array((yyval.declarator)->array, (yyvsp[0].expr)); }
#line 4669 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 345:
#line 1022 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = make_declarator(NULL); (yyval.declarator)->array = append_array((yyval.declarator)->array, (yyvsp[0].expr)); }
#line 4675 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 346:
#line 1024 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = make_declarator(NULL);
						  (yyval.declarator)->func_type = append_ptrchain_type((yyval.declarator)->type, type_new_function((yyvsp[-1].var_list)));
						  (yyval.declarator)->type = NULL;
						}
#line 4684 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 347:
#line 1029 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-3].declarator);
						  (yyval.declarator)->func_type = append_ptrchain_type((yyval.declarator)->type, type_new_function((yyvsp[-1].var_list)));
						  (yyval.declarator)->type = NULL;
						}
#line 4693 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 348:
#line 1038 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); (yyval.declarator)->type = append_ptrchain_type((yyval.declarator)->type, type_new_pointer(pointer_default, NULL, (yyvsp[-1].attr_list))); }
#line 4699 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 349:
#line 1039 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); (yyval.declarator)->type->attrs = append_attr((yyval.declarator)->type->attrs, make_attrp(ATTR_CALLCONV, (yyvsp[-1].str))); }
#line 4705 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 351:
#line 1046 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); (yyval.declarator)->type = append_ptrchain_type((yyval.declarator)->type, type_new_pointer(pointer_default, NULL, (yyvsp[-1].attr_list))); }
#line 4711 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 352:
#line 1047 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); (yyval.declarator)->type->attrs = append_attr((yyval.declarator)->type->attrs, make_attrp(ATTR_CALLCONV, (yyvsp[-1].str))); }
#line 4717 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 353:
#line 1051 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = make_declarator(NULL); }
#line 4723 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 355:
#line 1059 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = make_declarator((yyvsp[0].var)); }
#line 4729 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 356:
#line 1060 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-1].declarator); }
#line 4735 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 357:
#line 1061 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-1].declarator); (yyval.declarator)->array = append_array((yyval.declarator)->array, (yyvsp[0].expr)); }
#line 4741 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 358:
#line 1062 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = make_declarator(NULL); (yyval.declarator)->array = append_array((yyval.declarator)->array, (yyvsp[0].expr)); }
#line 4747 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 359:
#line 1064 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = make_declarator(NULL);
						  (yyval.declarator)->func_type = append_ptrchain_type((yyval.declarator)->type, type_new_function((yyvsp[-1].var_list)));
						  (yyval.declarator)->type = NULL;
						}
#line 4756 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 360:
#line 1069 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-3].declarator);
						  (yyval.declarator)->func_type = append_ptrchain_type((yyval.declarator)->type, type_new_function((yyvsp[-1].var_list)));
						  (yyval.declarator)->type = NULL;
						}
#line 4765 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 361:
#line 1076 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator_list) = append_declarator( NULL, (yyvsp[0].declarator) ); }
#line 4771 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 362:
#line 1077 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator_list) = append_declarator( (yyvsp[-2].declarator_list), (yyvsp[0].declarator) ); }
#line 4777 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 363:
#line 1080 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 4783 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 364:
#line 1081 "parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 4789 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 365:
#line 1084 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-1].declarator); (yyval.declarator)->bits = (yyvsp[0].expr);
						  if (!(yyval.declarator)->bits && !(yyval.declarator)->var->name)
						    error_loc("unnamed fields are not allowed\n");
						}
#line 4798 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 366:
#line 1091 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator_list) = append_declarator( NULL, (yyvsp[0].declarator) ); }
#line 4804 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 367:
#line 1093 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator_list) = append_declarator( (yyvsp[-2].declarator_list), (yyvsp[0].declarator) ); }
#line 4810 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 368:
#line 1097 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[0].declarator); }
#line 4816 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 369:
#line 1098 "parser.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-2].declarator); (yyvsp[-2].declarator)->var->eval = (yyvsp[0].expr); }
#line 4822 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 370:
#line 1102 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = THREADING_APARTMENT; }
#line 4828 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 371:
#line 1103 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = THREADING_NEUTRAL; }
#line 4834 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 372:
#line 1104 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = THREADING_SINGLE; }
#line 4840 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 373:
#line 1105 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = THREADING_FREE; }
#line 4846 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 374:
#line 1106 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = THREADING_BOTH; }
#line 4852 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 375:
#line 1110 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = FC_RP; }
#line 4858 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 376:
#line 1111 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = FC_UP; }
#line 4864 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 377:
#line 1112 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = FC_FP; }
#line 4870 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 378:
#line 1115 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_struct((yyvsp[-3].str), current_namespace, TRUE, (yyvsp[-1].var_list)); }
#line 4876 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 379:
#line 1118 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_void(); }
#line 4882 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 380:
#line 1119 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = find_type_or_error((yyvsp[0].str), 0); }
#line 4888 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 381:
#line 1120 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[0].type); }
#line 4894 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 382:
#line 1121 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[0].type); }
#line 4900 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 383:
#line 1122 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_enum((yyvsp[0].str), current_namespace, FALSE, NULL); }
#line 4906 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 384:
#line 1123 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[0].type); }
#line 4912 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 385:
#line 1124 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_struct((yyvsp[0].str), current_namespace, FALSE, NULL); }
#line 4918 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 386:
#line 1125 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[0].type); }
#line 4924 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 387:
#line 1126 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_nonencapsulated_union((yyvsp[0].str), FALSE, NULL); }
#line 4930 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 388:
#line 1127 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = make_safearray((yyvsp[-1].type)); }
#line 4936 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 389:
#line 1131 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-4].attr_list) = append_attribs((yyvsp[-4].attr_list), (yyvsp[-2].attr_list));
						  reg_typedefs((yyvsp[-1].declspec), (yyvsp[0].declarator_list), check_typedef_attrs((yyvsp[-4].attr_list)));
						  (yyval.statement) = make_statement_typedef((yyvsp[0].declarator_list));
						}
#line 4945 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 390:
#line 1138 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_nonencapsulated_union((yyvsp[-3].str), TRUE, (yyvsp[-1].var_list)); }
#line 4951 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 391:
#line 1141 "parser.y" /* yacc.c:1646  */
    { (yyval.type) = type_new_encapsulated_union((yyvsp[-8].str), (yyvsp[-5].var), (yyvsp[-3].var), (yyvsp[-1].var_list)); }
#line 4957 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 392:
#line 1145 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = MAKEVERSION((yyvsp[0].num), 0); }
#line 4963 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 393:
#line 1146 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = MAKEVERSION((yyvsp[-2].num), (yyvsp[0].num)); }
#line 4969 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 394:
#line 1147 "parser.y" /* yacc.c:1646  */
    { (yyval.num) = (yyvsp[0].num); }
#line 4975 "parser.tab.c" /* yacc.c:1646  */
    break;


#line 4979 "parser.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1150 "parser.y" /* yacc.c:1906  */


static void decl_builtin_basic(const char *name, enum type_basic_type type)
{
  type_t *t = type_new_basic(type);
  reg_type(t, name, NULL, 0);
}

static void decl_builtin_alias(const char *name, type_t *t)
{
  reg_type(type_new_alias(t, name), name, NULL, 0);
}

void init_types(void)
{
  decl_builtin_basic("byte", TYPE_BASIC_BYTE);
  decl_builtin_basic("wchar_t", TYPE_BASIC_WCHAR);
  decl_builtin_basic("float", TYPE_BASIC_FLOAT);
  decl_builtin_basic("double", TYPE_BASIC_DOUBLE);
  decl_builtin_basic("error_status_t", TYPE_BASIC_ERROR_STATUS_T);
  decl_builtin_basic("handle_t", TYPE_BASIC_HANDLE);
  decl_builtin_alias("boolean", type_new_basic(TYPE_BASIC_BYTE));
}

static str_list_t *append_str(str_list_t *list, char *str)
{
    struct str_list_entry_t *entry;

    if (!str) return list;
    if (!list)
    {
        list = xmalloc( sizeof(*list) );
        list_init( list );
    }
    entry = xmalloc( sizeof(*entry) );
    entry->str = str;
    list_add_tail( list, &entry->entry );
    return list;
}

static attr_list_t *append_attr(attr_list_t *list, attr_t *attr)
{
    attr_t *attr_existing;
    if (!attr) return list;
    if (!list)
    {
        list = xmalloc( sizeof(*list) );
        list_init( list );
    }
    LIST_FOR_EACH_ENTRY(attr_existing, list, attr_t, entry)
        if (attr_existing->type == attr->type)
        {
            parser_warning("duplicate attribute %s\n", get_attr_display_name(attr->type));
            /* use the last attribute, like MIDL does */
            list_remove(&attr_existing->entry);
            break;
        }
    list_add_tail( list, &attr->entry );
    return list;
}

static attr_list_t *move_attr(attr_list_t *dst, attr_list_t *src, enum attr_type type)
{
  attr_t *attr;
  if (!src) return dst;
  LIST_FOR_EACH_ENTRY(attr, src, attr_t, entry)
    if (attr->type == type)
    {
      list_remove(&attr->entry);
      return append_attr(dst, attr);
    }
  return dst;
}

static attr_list_t *append_attr_list(attr_list_t *new_list, attr_list_t *old_list)
{
  struct list *entry;

  if (!old_list) return new_list;

  while ((entry = list_head(old_list)))
  {
    attr_t *attr = LIST_ENTRY(entry, attr_t, entry);
    list_remove(entry);
    new_list = append_attr(new_list, attr);
  }
  return new_list;
}

static attr_list_t *dupattrs(const attr_list_t *list)
{
  attr_list_t *new_list;
  const attr_t *attr;

  if (!list) return NULL;

  new_list = xmalloc( sizeof(*list) );
  list_init( new_list );
  LIST_FOR_EACH_ENTRY(attr, list, const attr_t, entry)
  {
    attr_t *new_attr = xmalloc(sizeof(*new_attr));
    *new_attr = *attr;
    list_add_tail(new_list, &new_attr->entry);
  }
  return new_list;
}

static decl_spec_t *make_decl_spec(type_t *type, decl_spec_t *left, decl_spec_t *right, attr_t *attr, enum storage_class stgclass)
{
  decl_spec_t *declspec = left ? left : right;
  if (!declspec)
  {
    declspec = xmalloc(sizeof(*declspec));
    declspec->type = NULL;
    declspec->attrs = NULL;
    declspec->stgclass = STG_NONE;
  }
  declspec->type = type;
  if (left && declspec != left)
  {
    declspec->attrs = append_attr_list(declspec->attrs, left->attrs);
    if (declspec->stgclass == STG_NONE)
      declspec->stgclass = left->stgclass;
    else if (left->stgclass != STG_NONE)
      error_loc("only one storage class can be specified\n");
    assert(!left->type);
    free(left);
  }
  if (right && declspec != right)
  {
    declspec->attrs = append_attr_list(declspec->attrs, right->attrs);
    if (declspec->stgclass == STG_NONE)
      declspec->stgclass = right->stgclass;
    else if (right->stgclass != STG_NONE)
      error_loc("only one storage class can be specified\n");
    assert(!right->type);
    free(right);
  }

  declspec->attrs = append_attr(declspec->attrs, attr);
  if (declspec->stgclass == STG_NONE)
    declspec->stgclass = stgclass;
  else if (stgclass != STG_NONE)
    error_loc("only one storage class can be specified\n");

  /* apply attributes to type */
  if (type && declspec->attrs)
  {
    attr_list_t *attrs;
    declspec->type = duptype(type, 1);
    attrs = dupattrs(type->attrs);
    declspec->type->attrs = append_attr_list(attrs, declspec->attrs);
    declspec->attrs = NULL;
  }

  return declspec;
}

static attr_t *make_attr(enum attr_type type)
{
  attr_t *a = xmalloc(sizeof(attr_t));
  a->type = type;
  a->u.ival = 0;
  return a;
}

static attr_t *make_attrv(enum attr_type type, unsigned int val)
{
  attr_t *a = xmalloc(sizeof(attr_t));
  a->type = type;
  a->u.ival = val;
  return a;
}

static attr_t *make_attrp(enum attr_type type, void *val)
{
  attr_t *a = xmalloc(sizeof(attr_t));
  a->type = type;
  a->u.pval = val;
  return a;
}

static expr_list_t *append_expr(expr_list_t *list, expr_t *expr)
{
    if (!expr) return list;
    if (!list)
    {
        list = xmalloc( sizeof(*list) );
        list_init( list );
    }
    list_add_tail( list, &expr->entry );
    return list;
}

static array_dims_t *append_array(array_dims_t *list, expr_t *expr)
{
    if (!expr) return list;
    if (!list)
    {
        list = xmalloc( sizeof(*list) );
        list_init( list );
    }
    list_add_tail( list, &expr->entry );
    return list;
}

static struct list type_pool = LIST_INIT(type_pool);
typedef struct
{
  type_t data;
  struct list link;
} type_pool_node_t;

type_t *alloc_type(void)
{
  type_pool_node_t *node = xmalloc(sizeof *node);
  list_add_tail(&type_pool, &node->link);
  return &node->data;
}

void set_all_tfswrite(int val)
{
  type_pool_node_t *node;
  LIST_FOR_EACH_ENTRY(node, &type_pool, type_pool_node_t, link)
    node->data.tfswrite = val;
}

void clear_all_offsets(void)
{
  type_pool_node_t *node;
  LIST_FOR_EACH_ENTRY(node, &type_pool, type_pool_node_t, link)
    node->data.typestring_offset = node->data.ptrdesc = 0;
}

static void type_function_add_head_arg(type_t *type, var_t *arg)
{
    if (!type->details.function->args)
    {
        type->details.function->args = xmalloc( sizeof(*type->details.function->args) );
        list_init( type->details.function->args );
    }
    list_add_head( type->details.function->args, &arg->entry );
}

static int is_allowed_range_type(const type_t *type)
{
    switch (type_get_type(type))
    {
    case TYPE_ENUM:
        return TRUE;
    case TYPE_BASIC:
        switch (type_basic_get_type(type))
        {
        case TYPE_BASIC_INT8:
        case TYPE_BASIC_INT16:
        case TYPE_BASIC_INT32:
        case TYPE_BASIC_INT64:
        case TYPE_BASIC_INT:
        case TYPE_BASIC_INT3264:
        case TYPE_BASIC_BYTE:
        case TYPE_BASIC_CHAR:
        case TYPE_BASIC_WCHAR:
        case TYPE_BASIC_HYPER:
            return TRUE;
        case TYPE_BASIC_FLOAT:
        case TYPE_BASIC_DOUBLE:
        case TYPE_BASIC_ERROR_STATUS_T:
        case TYPE_BASIC_HANDLE:
            return FALSE;
        }
        return FALSE;
    default:
        return FALSE;
    }
}

static type_t *append_ptrchain_type(type_t *ptrchain, type_t *type)
{
  type_t *ptrchain_type;
  if (!ptrchain)
    return type;
  for (ptrchain_type = ptrchain; type_pointer_get_ref(ptrchain_type); ptrchain_type = type_pointer_get_ref(ptrchain_type))
    ;
  assert(ptrchain_type->type_type == TYPE_POINTER);
  ptrchain_type->details.pointer.ref = type;
  return ptrchain;
}

static warning_list_t *append_warning(warning_list_t *list, int num)
{
    warning_t *entry;

    if(!list)
    {
        list = xmalloc( sizeof(*list) );
        list_init( list );
    }
    entry = xmalloc( sizeof(*entry) );
    entry->num = num;
    list_add_tail( list, &entry->entry );
    return list;
}

static var_t *declare_var(attr_list_t *attrs, decl_spec_t *decl_spec, const declarator_t *decl,
                       int top)
{
  var_t *v = decl->var;
  expr_list_t *sizes = get_attrp(attrs, ATTR_SIZEIS);
  expr_list_t *lengs = get_attrp(attrs, ATTR_LENGTHIS);
  int sizeless;
  expr_t *dim;
  type_t **ptype;
  array_dims_t *arr = decl ? decl->array : NULL;
  type_t *func_type = decl ? decl->func_type : NULL;
  type_t *type = decl_spec->type;

  if (is_attr(type->attrs, ATTR_INLINE))
  {
    if (!func_type)
      error_loc("inline attribute applied to non-function type\n");
    else
    {
      type_t *t;
      /* move inline attribute from return type node to function node */
      for (t = func_type; is_ptr(t); t = type_pointer_get_ref(t))
        ;
      t->attrs = move_attr(t->attrs, type->attrs, ATTR_INLINE);
    }
  }

  /* add type onto the end of the pointers in pident->type */
  v->type = append_ptrchain_type(decl ? decl->type : NULL, type);
  v->stgclass = decl_spec->stgclass;
  v->attrs = attrs;

  /* check for pointer attribute being applied to non-pointer, non-array
   * type */
  if (!arr)
  {
    int ptr_attr = get_attrv(v->attrs, ATTR_POINTERTYPE);
    const type_t *ptr = NULL;
    /* pointer attributes on the left side of the type belong to the function
     * pointer, if one is being declared */
    type_t **pt = func_type ? &func_type : &v->type;
    for (ptr = *pt; ptr && !ptr_attr; )
    {
      ptr_attr = get_attrv(ptr->attrs, ATTR_POINTERTYPE);
      if (!ptr_attr && type_is_alias(ptr))
        ptr = type_alias_get_aliasee(ptr);
      else
        break;
    }
    if (is_ptr(ptr))
    {
      if (ptr_attr && ptr_attr != FC_UP &&
          type_get_type(type_pointer_get_ref(ptr)) == TYPE_INTERFACE)
          warning_loc_info(&v->loc_info,
                           "%s: pointer attribute applied to interface "
                           "pointer type has no effect\n", v->name);
      if (!ptr_attr && top && (*pt)->details.pointer.def_fc != FC_RP)
      {
        /* FIXME: this is a horrible hack to cope with the issue that we
         * store an offset to the typeformat string in the type object, but
         * two typeformat strings may be written depending on whether the
         * pointer is a toplevel parameter or not */
        *pt = duptype(*pt, 1);
      }
    }
    else if (ptr_attr)
       error_loc("%s: pointer attribute applied to non-pointer type\n", v->name);
  }

  if (is_attr(v->attrs, ATTR_STRING))
  {
    type_t *t = type;

    if (!is_ptr(v->type) && !arr)
      error_loc("'%s': [string] attribute applied to non-pointer, non-array type\n",
                v->name);

    while (is_ptr(t))
      t = type_pointer_get_ref(t);

    if (type_get_type(t) != TYPE_BASIC &&
        (get_basic_fc(t) != FC_CHAR &&
         get_basic_fc(t) != FC_BYTE &&
         get_basic_fc(t) != FC_WCHAR))
    {
      error_loc("'%s': [string] attribute is only valid on 'char', 'byte', or 'wchar_t' pointers and arrays\n",
                v->name);
    }
  }

  if (is_attr(v->attrs, ATTR_V1ENUM))
  {
    if (type_get_type_detect_alias(v->type) != TYPE_ENUM)
      error_loc("'%s': [v1_enum] attribute applied to non-enum type\n", v->name);
  }

  if (is_attr(v->attrs, ATTR_RANGE) && !is_allowed_range_type(v->type))
    error_loc("'%s': [range] attribute applied to non-integer type\n",
              v->name);

  ptype = &v->type;
  sizeless = FALSE;
  if (arr) LIST_FOR_EACH_ENTRY_REV(dim, arr, expr_t, entry)
  {
    if (sizeless)
      error_loc("%s: only the first array dimension can be unspecified\n", v->name);

    if (dim->is_const)
    {
      if (dim->cval <= 0)
        error_loc("%s: array dimension must be positive\n", v->name);

      /* FIXME: should use a type_memsize that allows us to pass in a pointer size */
      if (0)
      {
        unsigned int size = type_memsize(v->type);

        if (0xffffffffu / size < dim->cval)
          error_loc("%s: total array size is too large\n", v->name);
      }
    }
    else
      sizeless = TRUE;

    *ptype = type_new_array(NULL, *ptype, FALSE,
                            dim->is_const ? dim->cval : 0,
                            dim->is_const ? NULL : dim, NULL,
                            pointer_default);
  }

  ptype = &v->type;
  if (sizes) LIST_FOR_EACH_ENTRY(dim, sizes, expr_t, entry)
  {
    if (dim->type != EXPR_VOID)
    {
      if (is_array(*ptype))
      {
        if (!type_array_get_conformance(*ptype) ||
            type_array_get_conformance(*ptype)->type != EXPR_VOID)
          error_loc("%s: cannot specify size_is for an already sized array\n", v->name);
        else
          *ptype = type_new_array((*ptype)->name,
                                  type_array_get_element(*ptype), FALSE,
                                  0, dim, NULL, 0);
      }
      else if (is_ptr(*ptype))
        *ptype = type_new_array((*ptype)->name, type_pointer_get_ref(*ptype), TRUE,
                                0, dim, NULL, pointer_default);
      else
        error_loc("%s: size_is attribute applied to illegal type\n", v->name);
    }

    if (is_ptr(*ptype))
      ptype = &(*ptype)->details.pointer.ref;
    else if (is_array(*ptype))
      ptype = &(*ptype)->details.array.elem;
    else
      error_loc("%s: too many expressions in size_is attribute\n", v->name);
  }

  ptype = &v->type;
  if (lengs) LIST_FOR_EACH_ENTRY(dim, lengs, expr_t, entry)
  {
    if (dim->type != EXPR_VOID)
    {
      if (is_array(*ptype))
      {
        *ptype = type_new_array((*ptype)->name,
                                type_array_get_element(*ptype),
                                type_array_is_decl_as_ptr(*ptype),
                                type_array_get_dim(*ptype),
                                type_array_get_conformance(*ptype),
                                dim, type_array_get_ptr_default_fc(*ptype));
      }
      else
        error_loc("%s: length_is attribute applied to illegal type\n", v->name);
    }

    if (is_ptr(*ptype))
      ptype = &(*ptype)->details.pointer.ref;
    else if (is_array(*ptype))
      ptype = &(*ptype)->details.array.elem;
    else
      error_loc("%s: too many expressions in length_is attribute\n", v->name);
  }

  /* v->type is currently pointing to the type on the left-side of the
   * declaration, so we need to fix this up so that it is the return type of the
   * function and make v->type point to the function side of the declaration */
  if (func_type)
  {
    type_t *ft, *t;
    type_t *return_type = v->type;
    v->type = func_type;
    for (ft = v->type; is_ptr(ft); ft = type_pointer_get_ref(ft))
      ;
    assert(type_get_type_detect_alias(ft) == TYPE_FUNCTION);
    ft->details.function->retval = make_var(xstrdup("_RetVal"));
    ft->details.function->retval->type = return_type;
    /* move calling convention attribute, if present, from pointer nodes to
     * function node */
    for (t = v->type; is_ptr(t); t = type_pointer_get_ref(t))
      ft->attrs = move_attr(ft->attrs, t->attrs, ATTR_CALLCONV);
  }
  else
  {
    type_t *t;
    for (t = v->type; is_ptr(t); t = type_pointer_get_ref(t))
      if (is_attr(t->attrs, ATTR_CALLCONV))
        error_loc("calling convention applied to non-function-pointer type\n");
  }

  if (decl->bits)
    v->type = type_new_bitfield(v->type, decl->bits);

  return v;
}

static var_list_t *set_var_types(attr_list_t *attrs, decl_spec_t *decl_spec, declarator_list_t *decls)
{
  declarator_t *decl, *next;
  var_list_t *var_list = NULL;

  LIST_FOR_EACH_ENTRY_SAFE( decl, next, decls, declarator_t, entry )
  {
    var_t *var = declare_var(attrs, decl_spec, decl, 0);
    var_list = append_var(var_list, var);
    free(decl);
  }
  free(decl_spec);
  return var_list;
}

static ifref_list_t *append_ifref(ifref_list_t *list, ifref_t *iface)
{
    if (!iface) return list;
    if (!list)
    {
        list = xmalloc( sizeof(*list) );
        list_init( list );
    }
    list_add_tail( list, &iface->entry );
    return list;
}

static ifref_t *make_ifref(type_t *iface)
{
  ifref_t *l = xmalloc(sizeof(ifref_t));
  l->iface = iface;
  l->attrs = NULL;
  return l;
}

var_list_t *append_var(var_list_t *list, var_t *var)
{
    if (!var) return list;
    if (!list)
    {
        list = xmalloc( sizeof(*list) );
        list_init( list );
    }
    list_add_tail( list, &var->entry );
    return list;
}

static var_list_t *append_var_list(var_list_t *list, var_list_t *vars)
{
    if (!vars) return list;
    if (!list)
    {
        list = xmalloc( sizeof(*list) );
        list_init( list );
    }
    list_move_tail( list, vars );
    return list;
}

var_t *make_var(char *name)
{
  var_t *v = xmalloc(sizeof(var_t));
  v->name = name;
  v->type = NULL;
  v->attrs = NULL;
  v->eval = NULL;
  v->stgclass = STG_NONE;
  init_loc_info(&v->loc_info);
  return v;
}

static declarator_list_t *append_declarator(declarator_list_t *list, declarator_t *d)
{
  if (!d) return list;
  if (!list) {
    list = xmalloc(sizeof(*list));
    list_init(list);
  }
  list_add_tail(list, &d->entry);
  return list;
}

static declarator_t *make_declarator(var_t *var)
{
  declarator_t *d = xmalloc(sizeof(*d));
  d->var = var ? var : make_var(NULL);
  d->type = NULL;
  d->func_type = NULL;
  d->array = NULL;
  d->bits = NULL;
  return d;
}

static type_t *make_safearray(type_t *type)
{
  return type_new_array(NULL, type_new_alias(type, "SAFEARRAY"), TRUE, 0,
                        NULL, NULL, FC_RP);
}

static typelib_t *make_library(const char *name, const attr_list_t *attrs)
{
    typelib_t *typelib = xmalloc(sizeof(*typelib));
    typelib->name = xstrdup(name);
    typelib->attrs = attrs;
    list_init( &typelib->importlibs );
    return typelib;
}

static int hash_ident(const char *name)
{
  const char *p = name;
  int sum = 0;
  /* a simple sum hash is probably good enough */
  while (*p) {
    sum += *p;
    p++;
  }
  return sum & (HASHMAX-1);
}

/***** type repository *****/

static struct namespace *find_sub_namespace(struct namespace *namespace, const char *name)
{
  struct namespace *cur;

  LIST_FOR_EACH_ENTRY(cur, &namespace->children, struct namespace, entry) {
    if(!strcmp(cur->name, name))
      return cur;
  }

  return NULL;
}

static void push_namespace(const char *name)
{
  struct namespace *namespace;

  namespace = find_sub_namespace(current_namespace, name);
  if(!namespace) {
    namespace = xmalloc(sizeof(*namespace));
    namespace->name = xstrdup(name);
    namespace->parent = current_namespace;
    list_add_tail(&current_namespace->children, &namespace->entry);
    list_init(&namespace->children);
    memset(namespace->type_hash, 0, sizeof(namespace->type_hash));
  }

  current_namespace = namespace;
}

static void pop_namespace(const char *name)
{
  assert(!strcmp(current_namespace->name, name) && current_namespace->parent);
  current_namespace = current_namespace->parent;
}

struct rtype {
  const char *name;
  type_t *type;
  int t;
  struct rtype *next;
};

type_t *reg_type(type_t *type, const char *name, struct namespace *namespace, int t)
{
  struct rtype *nt;
  int hash;
  if (!name) {
    error_loc("registering named type without name\n");
    return type;
  }
  if (!namespace)
    namespace = &global_namespace;
  hash = hash_ident(name);
  nt = xmalloc(sizeof(struct rtype));
  nt->name = name;
  if (is_global_namespace(namespace))
    type->c_name = name;
  else
    type->c_name = format_namespace(namespace, "__x_", "_C", name);
  nt->type = type;
  nt->t = t;
  nt->next = namespace->type_hash[hash];
  namespace->type_hash[hash] = nt;
  if ((t == tsSTRUCT || t == tsUNION))
    fix_incomplete_types(type);
  return type;
}

static int is_incomplete(const type_t *t)
{
  return !t->defined &&
    (type_get_type_detect_alias(t) == TYPE_STRUCT ||
     type_get_type_detect_alias(t) == TYPE_UNION ||
     type_get_type_detect_alias(t) == TYPE_ENCAPSULATED_UNION);
}

void add_incomplete(type_t *t)
{
  struct typenode *tn = xmalloc(sizeof *tn);
  tn->type = t;
  list_add_tail(&incomplete_types, &tn->entry);
}

static void fix_type(type_t *t)
{
  if (type_is_alias(t) && is_incomplete(t)) {
    type_t *ot = type_alias_get_aliasee(t);
    fix_type(ot);
    if (type_get_type_detect_alias(ot) == TYPE_STRUCT ||
        type_get_type_detect_alias(ot) == TYPE_UNION ||
        type_get_type_detect_alias(ot) == TYPE_ENCAPSULATED_UNION)
      t->details.structure = ot->details.structure;
    t->defined = ot->defined;
  }
}

static void fix_incomplete(void)
{
  struct typenode *tn, *next;

  LIST_FOR_EACH_ENTRY_SAFE(tn, next, &incomplete_types, struct typenode, entry) {
    fix_type(tn->type);
    list_remove(&tn->entry);
    free(tn);
  }
}

static void fix_incomplete_types(type_t *complete_type)
{
  struct typenode *tn, *next;

  LIST_FOR_EACH_ENTRY_SAFE(tn, next, &incomplete_types, struct typenode, entry)
  {
    if (type_is_equal(complete_type, tn->type))
    {
      tn->type->details.structure = complete_type->details.structure;
      list_remove(&tn->entry);
      free(tn);
    }
  }
}

static type_t *reg_typedefs(decl_spec_t *decl_spec, declarator_list_t *decls, attr_list_t *attrs)
{
  const declarator_t *decl;
  type_t *type = decl_spec->type;

  if (is_attr(attrs, ATTR_UUID) && !is_attr(attrs, ATTR_PUBLIC))
    attrs = append_attr( attrs, make_attr(ATTR_PUBLIC) );

  /* We must generate names for tagless enum, struct or union.
     Typedef-ing a tagless enum, struct or union means we want the typedef
     to be included in a library hence the public attribute.  */
  if (type_get_type_detect_alias(type) == TYPE_ENUM ||
      type_get_type_detect_alias(type) == TYPE_STRUCT ||
      type_get_type_detect_alias(type) == TYPE_UNION ||
      type_get_type_detect_alias(type) == TYPE_ENCAPSULATED_UNION)
  {
    if (!type->name)
      type->name = gen_name();

    /* replace existing attributes when generating a typelib */
    if (do_typelib)
        type->attrs = attrs;
  }

  LIST_FOR_EACH_ENTRY( decl, decls, const declarator_t, entry )
  {

    if (decl->var->name) {
      type_t *cur;
      var_t *name;

      cur = find_type(decl->var->name, current_namespace, 0);

      /*
       * MIDL allows shadowing types that are declared in imported files.
       * We don't throw an error in this case and instead add a new type
       * (which is earlier on the list in hash table, so it will be used
       * instead of shadowed type).
       *
       * FIXME: We may consider string separated type tables for each input
       *        for cleaner solution.
       */
      if (cur && input_name == cur->loc_info.input_name)
          error_loc("%s: redefinition error; original definition was at %s:%d\n",
                    cur->name, cur->loc_info.input_name,
                    cur->loc_info.line_number);

      name = declare_var(attrs, decl_spec, decl, 0);
      cur = type_new_alias(name->type, name->name);
      cur->attrs = attrs;

      if (is_incomplete(cur))
        add_incomplete(cur);
      reg_type(cur, cur->name, current_namespace, 0);
    }
  }
  return type;
}

type_t *find_type(const char *name, struct namespace *namespace, int t)
{
  struct rtype *cur;

  if(namespace && namespace != &global_namespace) {
    for(cur = namespace->type_hash[hash_ident(name)]; cur; cur = cur->next) {
      if(cur->t == t && !strcmp(cur->name, name))
        return cur->type;
    }
  }
  for(cur = global_namespace.type_hash[hash_ident(name)]; cur; cur = cur->next) {
    if(cur->t == t && !strcmp(cur->name, name))
      return cur->type;
  }
  return NULL;
}

static type_t *find_type_or_error(const char *name, int t)
{
  type_t *type = find_type(name, NULL, t);
  if (!type) {
    error_loc("type '%s' not found\n", name);
    return NULL;
  }
  return type;
}

static type_t *find_type_or_error2(char *name, int t)
{
  type_t *tp = find_type_or_error(name, t);
  free(name);
  return tp;
}

int is_type(const char *name)
{
  return find_type(name, current_namespace, 0) != NULL;
}

type_t *get_type(enum type_type type, char *name, struct namespace *namespace, int t)
{
  type_t *tp;
  if (!namespace)
    namespace = &global_namespace;
  if (name) {
    tp = find_type(name, namespace, t);
    if (tp) {
      free(name);
      return tp;
    }
  }
  tp = make_type(type);
  tp->name = name;
  tp->namespace = namespace;
  if (!name) return tp;
  return reg_type(tp, name, namespace, t);
}

/***** constant repository *****/

struct rconst {
  char *name;
  var_t *var;
  struct rconst *next;
};

struct rconst *const_hash[HASHMAX];

static var_t *reg_const(var_t *var)
{
  struct rconst *nc;
  int hash;
  if (!var->name) {
    error_loc("registering constant without name\n");
    return var;
  }
  hash = hash_ident(var->name);
  nc = xmalloc(sizeof(struct rconst));
  nc->name = var->name;
  nc->var = var;
  nc->next = const_hash[hash];
  const_hash[hash] = nc;
  return var;
}

var_t *find_const(const char *name, int f)
{
  struct rconst *cur = const_hash[hash_ident(name)];
  while (cur && strcmp(cur->name, name))
    cur = cur->next;
  if (!cur) {
    if (f) error_loc("constant '%s' not found\n", name);
    return NULL;
  }
  return cur->var;
}

static char *gen_name(void)
{
  static const char format[] = "__WIDL_%s_generated_name_%08lX";
  static unsigned long n = 0;
  static const char *file_id;
  static size_t size;
  char *name;

  if (! file_id)
  {
    char *dst = dup_basename(input_idl_name, ".idl");
    file_id = dst;

    for (; *dst; ++dst)
      if (! isalnum((unsigned char) *dst))
        *dst = '_';

    size = sizeof format - 7 + strlen(file_id) + 8;
  }

  name = xmalloc(size);
  sprintf(name, format, file_id, n++);
  return name;
}

struct allowed_attr
{
    unsigned int dce_compatible : 1;
    unsigned int acf : 1;
    unsigned int on_interface : 1;
    unsigned int on_function : 1;
    unsigned int on_arg : 1;
    unsigned int on_type : 1;
    unsigned int on_enum : 1;
    unsigned int on_struct : 2;
    unsigned int on_union : 1;
    unsigned int on_field : 1;
    unsigned int on_library : 1;
    unsigned int on_dispinterface : 1;
    unsigned int on_module : 1;
    unsigned int on_coclass : 1;
    const char *display_name;
};

struct allowed_attr allowed_attr[] =
{
    /* attr                        { D ACF I Fn ARG T En St Un Fi  L  DI M  C  <display name> } */
    /* ATTR_AGGREGATABLE */        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, "aggregatable" },
    /* ATTR_ANNOTATION */          { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "annotation" },
    /* ATTR_APPOBJECT */           { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, "appobject" },
    /* ATTR_ASYNC */               { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "async" },
    /* ATTR_ASYNCUUID */           { 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, "async_uuid" },
    /* ATTR_AUTO_HANDLE */         { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "auto_handle" },
    /* ATTR_BINDABLE */            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "bindable" },
    /* ATTR_BROADCAST */           { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "broadcast" },
    /* ATTR_CALLAS */              { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "call_as" },
    /* ATTR_CALLCONV */            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL },
    /* ATTR_CASE */                { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, "case" },
    /* ATTR_CODE */                { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "code" },
    /* ATTR_COMMSTATUS */          { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "comm_status" },
    /* ATTR_CONST */               { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "const" },
    /* ATTR_CONTEXTHANDLE */       { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, "context_handle" },
    /* ATTR_CONTROL */             { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, "control" },
    /* ATTR_DECODE */              { 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, "decode" },
    /* ATTR_DEFAULT */             { 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, "default" },
    /* ATTR_DEFAULTBIND */         { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "defaultbind" },
    /* ATTR_DEFAULTCOLLELEM */     { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "defaultcollelem" },
    /* ATTR_DEFAULTVALUE */        { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "defaultvalue" },
    /* ATTR_DEFAULTVTABLE */       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, "defaultvtable" },
 /* ATTR_DISABLECONSISTENCYCHECK */{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "disable_consistency_check" },
    /* ATTR_DISPINTERFACE */       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL },
    /* ATTR_DISPLAYBIND */         { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "displaybind" },
    /* ATTR_DLLNAME */             { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, "dllname" },
    /* ATTR_DUAL */                { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "dual" },
    /* ATTR_ENABLEALLOCATE */      { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "enable_allocate" },
    /* ATTR_ENCODE */              { 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, "encode" },
    /* ATTR_ENDPOINT */            { 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "endpoint" },
    /* ATTR_ENTRY */               { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "entry" },
    /* ATTR_EXPLICIT_HANDLE */     { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "explicit_handle" },
    /* ATTR_FAULTSTATUS */         { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "fault_status" },
    /* ATTR_FORCEALLOCATE */       { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "force_allocate" },
    /* ATTR_HANDLE */              { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, "handle" },
    /* ATTR_HELPCONTEXT */         { 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, "helpcontext" },
    /* ATTR_HELPFILE */            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, "helpfile" },
    /* ATTR_HELPSTRING */          { 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, "helpstring" },
    /* ATTR_HELPSTRINGCONTEXT */   { 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, "helpstringcontext" },
    /* ATTR_HELPSTRINGDLL */       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, "helpstringdll" },
    /* ATTR_HIDDEN */              { 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, "hidden" },
    /* ATTR_ID */                  { 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, "id" },
    /* ATTR_IDEMPOTENT */          { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "idempotent" },
    /* ATTR_IGNORE */              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, "ignore" },
    /* ATTR_IIDIS */               { 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, "iid_is" },
    /* ATTR_IMMEDIATEBIND */       { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "immediatebind" },
    /* ATTR_IMPLICIT_HANDLE */     { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "implicit_handle" },
    /* ATTR_IN */                  { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "in" },
    /* ATTR_INLINE */              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "inline" },
    /* ATTR_INPUTSYNC */           { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "inputsync" },
    /* ATTR_LENGTHIS */            { 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, "length_is" },
    /* ATTR_LIBLCID */             { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, "lcid" },
    /* ATTR_LICENSED */            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, "licensed" },
    /* ATTR_LOCAL */               { 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "local" },
    /* ATTR_MAYBE */               { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "maybe" },
    /* ATTR_MESSAGE */             { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "message" },
    /* ATTR_NOCODE */              { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "nocode" },
    /* ATTR_NONBROWSABLE */        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "nonbrowsable" },
    /* ATTR_NONCREATABLE */        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, "noncreatable" },
    /* ATTR_NONEXTENSIBLE */       { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "nonextensible" },
    /* ATTR_NOTIFY */              { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "notify" },
    /* ATTR_NOTIFYFLAG */          { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "notify_flag" },
    /* ATTR_OBJECT */              { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "object" },
    /* ATTR_ODL */                 { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, "odl" },
    /* ATTR_OLEAUTOMATION */       { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "oleautomation" },
    /* ATTR_OPTIMIZE */            { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "optimize" },
    /* ATTR_OPTIONAL */            { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "optional" },
    /* ATTR_OUT */                 { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "out" },
    /* ATTR_PARAMLCID */           { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "lcid" },
    /* ATTR_PARTIALIGNORE */       { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "partial_ignore" },
    /* ATTR_POINTERDEFAULT */      { 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "pointer_default" },
    /* ATTR_POINTERTYPE */         { 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, "ref, unique or ptr" },
    /* ATTR_PROGID */              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, "progid" },
    /* ATTR_PROPGET */             { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "propget" },
    /* ATTR_PROPPUT */             { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "propput" },
    /* ATTR_PROPPUTREF */          { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "propputref" },
    /* ATTR_PROXY */               { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "proxy" },
    /* ATTR_PUBLIC */              { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, "public" },
    /* ATTR_RANGE */               { 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, "range" },
    /* ATTR_READONLY */            { 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, "readonly" },
    /* ATTR_REPRESENTAS */         { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, "represent_as" },
    /* ATTR_REQUESTEDIT */         { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "requestedit" },
    /* ATTR_RESTRICTED */          { 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, "restricted" },
    /* ATTR_RETVAL */              { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "retval" },
    /* ATTR_SIZEIS */              { 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, "size_is" },
    /* ATTR_SOURCE */              { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, "source" },
    /* ATTR_STRICTCONTEXTHANDLE */ { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "strict_context_handle" },
    /* ATTR_STRING */              { 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, "string" },
    /* ATTR_SWITCHIS */            { 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, "switch_is" },
    /* ATTR_SWITCHTYPE */          { 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, "switch_type" },
    /* ATTR_THREADING */           { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, "threading" },
    /* ATTR_TRANSMITAS */          { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, "transmit_as" },
    /* ATTR_UIDEFAULT */           { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "uidefault" },
    /* ATTR_USESGETLASTERROR */    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "usesgetlasterror" },
    /* ATTR_USERMARSHAL */         { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, "user_marshal" },
    /* ATTR_UUID */                { 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, "uuid" },
    /* ATTR_V1ENUM */              { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, "v1_enum" },
    /* ATTR_VARARG */              { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "vararg" },
    /* ATTR_VERSION */             { 1, 0, 1, 0, 0, 1, 1, 2, 0, 0, 1, 0, 0, 1, "version" },
    /* ATTR_VIPROGID */            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, "vi_progid" },
    /* ATTR_WIREMARSHAL */         { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, "wire_marshal" },
};

const char *get_attr_display_name(enum attr_type type)
{
    return allowed_attr[type].display_name;
}

static attr_list_t *check_iface_attrs(const char *name, attr_list_t *attrs)
{
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!allowed_attr[attr->type].on_interface)
      error_loc("inapplicable attribute %s for interface %s\n",
                allowed_attr[attr->type].display_name, name);
    if (attr->type == ATTR_IMPLICIT_HANDLE)
    {
        const var_t *var = attr->u.pval;
        if (type_get_type( var->type) == TYPE_BASIC &&
            type_basic_get_type( var->type ) == TYPE_BASIC_HANDLE)
            continue;
        if (is_aliaschain_attr( var->type, ATTR_HANDLE ))
            continue;
      error_loc("attribute %s requires a handle type in interface %s\n",
                allowed_attr[attr->type].display_name, name);
    }
  }
  return attrs;
}

static attr_list_t *check_function_attrs(const char *name, attr_list_t *attrs)
{
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!allowed_attr[attr->type].on_function)
      error_loc("inapplicable attribute %s for function %s\n",
                allowed_attr[attr->type].display_name, name);
  }
  return attrs;
}

static void check_arg_attrs(const var_t *arg)
{
  const attr_t *attr;

  if (arg->attrs)
  {
    LIST_FOR_EACH_ENTRY(attr, arg->attrs, const attr_t, entry)
    {
      if (!allowed_attr[attr->type].on_arg)
        error_loc("inapplicable attribute %s for argument %s\n",
                  allowed_attr[attr->type].display_name, arg->name);
    }
  }
}

static attr_list_t *check_typedef_attrs(attr_list_t *attrs)
{
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!allowed_attr[attr->type].on_type)
      error_loc("inapplicable attribute %s for typedef\n",
                allowed_attr[attr->type].display_name);
  }
  return attrs;
}

static attr_list_t *check_enum_attrs(attr_list_t *attrs)
{
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!allowed_attr[attr->type].on_enum)
      error_loc("inapplicable attribute %s for enum\n",
                allowed_attr[attr->type].display_name);
  }
  return attrs;
}

static attr_list_t *check_struct_attrs(attr_list_t *attrs)
{
  int mask = winrt_mode ? 3 : 1;
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!(allowed_attr[attr->type].on_struct & mask))
      error_loc("inapplicable attribute %s for struct\n",
                allowed_attr[attr->type].display_name);
  }
  return attrs;
}

static attr_list_t *check_union_attrs(attr_list_t *attrs)
{
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!allowed_attr[attr->type].on_union)
      error_loc("inapplicable attribute %s for union\n",
                allowed_attr[attr->type].display_name);
  }
  return attrs;
}

static attr_list_t *check_field_attrs(const char *name, attr_list_t *attrs)
{
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!allowed_attr[attr->type].on_field)
      error_loc("inapplicable attribute %s for field %s\n",
                allowed_attr[attr->type].display_name, name);
  }
  return attrs;
}

static attr_list_t *check_library_attrs(const char *name, attr_list_t *attrs)
{
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!allowed_attr[attr->type].on_library)
      error_loc("inapplicable attribute %s for library %s\n",
                allowed_attr[attr->type].display_name, name);
  }
  return attrs;
}

static attr_list_t *check_dispiface_attrs(const char *name, attr_list_t *attrs)
{
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!allowed_attr[attr->type].on_dispinterface)
      error_loc("inapplicable attribute %s for dispinterface %s\n",
                allowed_attr[attr->type].display_name, name);
  }
  return attrs;
}

static attr_list_t *check_module_attrs(const char *name, attr_list_t *attrs)
{
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!allowed_attr[attr->type].on_module)
      error_loc("inapplicable attribute %s for module %s\n",
                allowed_attr[attr->type].display_name, name);
  }
  return attrs;
}

static attr_list_t *check_coclass_attrs(const char *name, attr_list_t *attrs)
{
  const attr_t *attr;
  if (!attrs) return attrs;
  LIST_FOR_EACH_ENTRY(attr, attrs, const attr_t, entry)
  {
    if (!allowed_attr[attr->type].on_coclass)
      error_loc("inapplicable attribute %s for coclass %s\n",
                allowed_attr[attr->type].display_name, name);
  }
  return attrs;
}

static int is_allowed_conf_type(const type_t *type)
{
    switch (type_get_type(type))
    {
    case TYPE_ENUM:
        return TRUE;
    case TYPE_BASIC:
        switch (type_basic_get_type(type))
        {
        case TYPE_BASIC_INT8:
        case TYPE_BASIC_INT16:
        case TYPE_BASIC_INT32:
        case TYPE_BASIC_INT64:
        case TYPE_BASIC_INT:
        case TYPE_BASIC_CHAR:
        case TYPE_BASIC_HYPER:
        case TYPE_BASIC_BYTE:
        case TYPE_BASIC_WCHAR:
            return TRUE;
        default:
            return FALSE;
        }
    case TYPE_ALIAS:
        /* shouldn't get here because of type_get_type call above */
        assert(0);
        /* fall through */
    case TYPE_STRUCT:
    case TYPE_UNION:
    case TYPE_ENCAPSULATED_UNION:
    case TYPE_ARRAY:
    case TYPE_POINTER:
    case TYPE_VOID:
    case TYPE_MODULE:
    case TYPE_COCLASS:
    case TYPE_FUNCTION:
    case TYPE_INTERFACE:
    case TYPE_BITFIELD:
        return FALSE;
    }
    return FALSE;
}

static int is_ptr_guid_type(const type_t *type)
{
    /* first, make sure it is a pointer to something */
    if (!is_ptr(type)) return FALSE;

    /* second, make sure it is a pointer to something of size sizeof(GUID),
     * i.e. 16 bytes */
    return (type_memsize(type_pointer_get_ref(type)) == 16);
}

static void check_conformance_expr_list(const char *attr_name, const var_t *arg, const type_t *container_type, expr_list_t *expr_list)
{
    expr_t *dim;
    struct expr_loc expr_loc;
    expr_loc.v = arg;
    expr_loc.attr = attr_name;
    if (expr_list) LIST_FOR_EACH_ENTRY(dim, expr_list, expr_t, entry)
    {
        if (dim->type != EXPR_VOID)
        {
            const type_t *expr_type = expr_resolve_type(&expr_loc, container_type, dim);
            if (!is_allowed_conf_type(expr_type))
                error_loc_info(&arg->loc_info, "expression must resolve to integral type <= 32bits for attribute %s\n",
                               attr_name);
        }
    }
}

static void check_remoting_fields(const var_t *var, type_t *type);

/* checks that properties common to fields and arguments are consistent */
static void check_field_common(const type_t *container_type,
                               const char *container_name, const var_t *arg)
{
    type_t *type = arg->type;
    int more_to_do;
    const char *container_type_name;
    const char *var_type;

    switch (type_get_type(container_type))
    {
    case TYPE_STRUCT:
        container_type_name = "struct";
        var_type = "field";
        break;
    case TYPE_UNION:
        container_type_name = "union";
        var_type = "arm";
        break;
    case TYPE_ENCAPSULATED_UNION:
        container_type_name = "encapsulated union";
        var_type = "arm";
        break;
    case TYPE_FUNCTION:
        container_type_name = "function";
        var_type = "parameter";
        break;
    default:
        /* should be no other container types */
        assert(0);
        return;
    }

    if (is_attr(arg->attrs, ATTR_LENGTHIS) &&
        (is_attr(arg->attrs, ATTR_STRING) || is_aliaschain_attr(arg->type, ATTR_STRING)))
        error_loc_info(&arg->loc_info,
                       "string and length_is specified for argument %s are mutually exclusive attributes\n",
                       arg->name);

    if (is_attr(arg->attrs, ATTR_SIZEIS))
    {
        expr_list_t *size_is_exprs = get_attrp(arg->attrs, ATTR_SIZEIS);
        check_conformance_expr_list("size_is", arg, container_type, size_is_exprs);
    }
    if (is_attr(arg->attrs, ATTR_LENGTHIS))
    {
        expr_list_t *length_is_exprs = get_attrp(arg->attrs, ATTR_LENGTHIS);
        check_conformance_expr_list("length_is", arg, container_type, length_is_exprs);
    }
    if (is_attr(arg->attrs, ATTR_IIDIS))
    {
        struct expr_loc expr_loc;
        expr_t *expr = get_attrp(arg->attrs, ATTR_IIDIS);
        if (expr->type != EXPR_VOID)
        {
            const type_t *expr_type;
            expr_loc.v = arg;
            expr_loc.attr = "iid_is";
            expr_type = expr_resolve_type(&expr_loc, container_type, expr);
            if (!expr_type || !is_ptr_guid_type(expr_type))
                error_loc_info(&arg->loc_info, "expression must resolve to pointer to GUID type for attribute iid_is\n");
        }
    }
    if (is_attr(arg->attrs, ATTR_SWITCHIS))
    {
        struct expr_loc expr_loc;
        expr_t *expr = get_attrp(arg->attrs, ATTR_SWITCHIS);
        if (expr->type != EXPR_VOID)
        {
            const type_t *expr_type;
            expr_loc.v = arg;
            expr_loc.attr = "switch_is";
            expr_type = expr_resolve_type(&expr_loc, container_type, expr);
            if (!expr_type || !is_allowed_conf_type(expr_type))
                error_loc_info(&arg->loc_info, "expression must resolve to integral type <= 32bits for attribute %s\n",
                               expr_loc.attr);
        }
    }

    do
    {
        more_to_do = FALSE;

        switch (typegen_detect_type(type, arg->attrs, TDT_IGNORE_STRINGS))
        {
        case TGT_STRUCT:
        case TGT_UNION:
            check_remoting_fields(arg, type);
            break;
        case TGT_INVALID:
        {
            const char *reason = "is invalid";
            switch (type_get_type(type))
            {
            case TYPE_VOID:
                reason = "cannot derive from void *";
                break;
            case TYPE_FUNCTION:
                reason = "cannot be a function pointer";
                break;
            case TYPE_BITFIELD:
                reason = "cannot be a bit-field";
                break;
            case TYPE_COCLASS:
                reason = "cannot be a class";
                break;
            case TYPE_INTERFACE:
                reason = "cannot be a non-pointer to an interface";
                break;
            case TYPE_MODULE:
                reason = "cannot be a module";
                break;
            default:
                break;
            }
            error_loc_info(&arg->loc_info, "%s \'%s\' of %s \'%s\' %s\n",
                           var_type, arg->name, container_type_name, container_name, reason);
            break;
        }
        case TGT_CTXT_HANDLE:
        case TGT_CTXT_HANDLE_POINTER:
            if (type_get_type(container_type) != TYPE_FUNCTION)
                error_loc_info(&arg->loc_info,
                               "%s \'%s\' of %s \'%s\' cannot be a context handle\n",
                               var_type, arg->name, container_type_name,
                               container_name);
            break;
        case TGT_STRING:
        {
            const type_t *t = type;
            while (is_ptr(t))
                t = type_pointer_get_ref(t);
            if (is_aliaschain_attr(t, ATTR_RANGE))
                warning_loc_info(&arg->loc_info, "%s: range not verified for a string of ranged types\n", arg->name);
            break;
        }
        case TGT_POINTER:
            type = type_pointer_get_ref(type);
            more_to_do = TRUE;
            break;
        case TGT_ARRAY:
            type = type_array_get_element(type);
            more_to_do = TRUE;
            break;
        case TGT_USER_TYPE:
        case TGT_IFACE_POINTER:
        case TGT_BASIC:
        case TGT_ENUM:
        case TGT_RANGE:
            /* nothing to do */
            break;
        }
    } while (more_to_do);
}

static void check_remoting_fields(const var_t *var, type_t *type)
{
    const var_t *field;
    const var_list_t *fields = NULL;

    type = type_get_real_type(type);

    if (type->checked)
        return;

    type->checked = TRUE;

    if (type_get_type(type) == TYPE_STRUCT)
    {
        if (type_is_complete(type))
            fields = type_struct_get_fields(type);
        else
            error_loc_info(&var->loc_info, "undefined type declaration %s\n", type->name);
    }
    else if (type_get_type(type) == TYPE_UNION || type_get_type(type) == TYPE_ENCAPSULATED_UNION)
        fields = type_union_get_cases(type);

    if (fields) LIST_FOR_EACH_ENTRY( field, fields, const var_t, entry )
        if (field->type) check_field_common(type, type->name, field);
}

/* checks that arguments for a function make sense for marshalling and unmarshalling */
static void check_remoting_args(const var_t *func)
{
    const char *funcname = func->name;
    const var_t *arg;

    if (func->type->details.function->args) LIST_FOR_EACH_ENTRY( arg, func->type->details.function->args, const var_t, entry )
    {
        const type_t *type = arg->type;

        /* check that [out] parameters have enough pointer levels */
        if (is_attr(arg->attrs, ATTR_OUT))
        {
            switch (typegen_detect_type(type, arg->attrs, TDT_ALL_TYPES))
            {
            case TGT_BASIC:
            case TGT_ENUM:
            case TGT_RANGE:
            case TGT_STRUCT:
            case TGT_UNION:
            case TGT_CTXT_HANDLE:
            case TGT_USER_TYPE:
                error_loc_info(&arg->loc_info, "out parameter \'%s\' of function \'%s\' is not a pointer\n", arg->name, funcname);
                break;
            case TGT_IFACE_POINTER:
                error_loc_info(&arg->loc_info, "out interface pointer \'%s\' of function \'%s\' is not a double pointer\n", arg->name, funcname);
                break;
            case TGT_STRING:
                if (is_array(type))
                {
                    /* needs conformance or fixed dimension */
                    if (type_array_has_conformance(type) &&
                        type_array_get_conformance(type)->type != EXPR_VOID) break;
                    if (!type_array_has_conformance(type) && type_array_get_dim(type)) break;
                }
                if (is_attr( arg->attrs, ATTR_IN )) break;
                error_loc_info(&arg->loc_info, "out parameter \'%s\' of function \'%s\' cannot be an unsized string\n", arg->name, funcname);
                break;
            case TGT_INVALID:
                /* already error'd before we get here */
            case TGT_CTXT_HANDLE_POINTER:
            case TGT_POINTER:
            case TGT_ARRAY:
                /* OK */
                break;
            }
        }

        check_field_common(func->type, funcname, arg);
    }

    if (type_get_type(type_function_get_rettype(func->type)) != TYPE_VOID)
    {
        var_t var;
        var = *func;
        var.type = type_function_get_rettype(func->type);
        var.name = xstrdup("return value");
        check_field_common(func->type, funcname, &var);
        free(var.name);
    }
}

static void add_explicit_handle_if_necessary(const type_t *iface, var_t *func)
{
    unsigned char explicit_fc, implicit_fc;

    /* check for a defined binding handle */
    if (!get_func_handle_var( iface, func, &explicit_fc, &implicit_fc ) || !explicit_fc)
    {
        /* no explicit handle specified so add
         * "[in] handle_t IDL_handle" as the first parameter to the
         * function */
        var_t *idl_handle = make_var(xstrdup("IDL_handle"));
        idl_handle->attrs = append_attr(NULL, make_attr(ATTR_IN));
        idl_handle->type = find_type_or_error("handle_t", 0);
        type_function_add_head_arg(func->type, idl_handle);
    }
}

static void check_functions(const type_t *iface, int is_inside_library)
{
    const statement_t *stmt;
    if (is_attr(iface->attrs, ATTR_EXPLICIT_HANDLE))
    {
        STATEMENTS_FOR_EACH_FUNC( stmt, type_iface_get_stmts(iface) )
        {
            var_t *func = stmt->u.var;
            add_explicit_handle_if_necessary(iface, func);
        }
    }
    if (!is_inside_library && !is_attr(iface->attrs, ATTR_LOCAL))
    {
        STATEMENTS_FOR_EACH_FUNC( stmt, type_iface_get_stmts(iface) )
        {
            const var_t *func = stmt->u.var;
            if (!is_attr(func->attrs, ATTR_LOCAL))
                check_remoting_args(func);
        }
    }
}

static void check_statements(const statement_list_t *stmts, int is_inside_library)
{
    const statement_t *stmt;

    if (stmts) LIST_FOR_EACH_ENTRY(stmt, stmts, const statement_t, entry)
    {
        switch(stmt->type) {
        case STMT_LIBRARY:
            check_statements(stmt->u.lib->stmts, TRUE);
            break;
        case STMT_TYPE:
            switch(type_get_type(stmt->u.type)) {
            case TYPE_INTERFACE:
                check_functions(stmt->u.type, is_inside_library);
                break;
            case TYPE_COCLASS:
                if(winrt_mode)
                    error_loc("coclass is not allowed in Windows Runtime mode\n");
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

static void check_all_user_types(const statement_list_t *stmts)
{
  const statement_t *stmt;

  if (stmts) LIST_FOR_EACH_ENTRY(stmt, stmts, const statement_t, entry)
  {
    if (stmt->type == STMT_LIBRARY)
      check_all_user_types(stmt->u.lib->stmts);
    else if (stmt->type == STMT_TYPE && type_get_type(stmt->u.type) == TYPE_INTERFACE &&
             !is_local(stmt->u.type->attrs))
    {
      const statement_t *stmt_func;
      STATEMENTS_FOR_EACH_FUNC(stmt_func, type_iface_get_stmts(stmt->u.type)) {
        const var_t *func = stmt_func->u.var;
        check_for_additional_prototype_types(func->type->details.function->args);
      }
    }
  }
}

int is_valid_uuid(const char *s)
{
  int i;

  for (i = 0; i < 36; ++i)
    if (i == 8 || i == 13 || i == 18 || i == 23)
    {
      if (s[i] != '-')
        return FALSE;
    }
    else
      if (!isxdigit(s[i]))
        return FALSE;

  return s[i] == '\0';
}

static statement_t *make_statement(enum statement_type type)
{
    statement_t *stmt = xmalloc(sizeof(*stmt));
    stmt->type = type;
    return stmt;
}

static statement_t *make_statement_type_decl(type_t *type)
{
    statement_t *stmt = make_statement(STMT_TYPE);
    stmt->u.type = type;
    return stmt;
}

static statement_t *make_statement_reference(type_t *type)
{
    statement_t *stmt = make_statement(STMT_TYPEREF);
    stmt->u.type = type;
    return stmt;
}

static statement_t *make_statement_declaration(var_t *var)
{
    statement_t *stmt = make_statement(STMT_DECLARATION);
    stmt->u.var = var;
    if (var->stgclass == STG_EXTERN && var->eval)
        warning("'%s' initialised and declared extern\n", var->name);
    if (is_const_decl(var))
    {
        if (var->eval)
            reg_const(var);
    }
    else if (type_get_type(var->type) == TYPE_FUNCTION)
        check_function_attrs(var->name, var->attrs);
    else if (var->stgclass == STG_NONE || var->stgclass == STG_REGISTER)
        error_loc("instantiation of data is illegal\n");
    return stmt;
}

static statement_t *make_statement_library(typelib_t *typelib)
{
    statement_t *stmt = make_statement(STMT_LIBRARY);
    stmt->u.lib = typelib;
    return stmt;
}

static statement_t *make_statement_pragma(const char *str)
{
    statement_t *stmt = make_statement(STMT_PRAGMA);
    stmt->u.str = str;
    return stmt;
}

static statement_t *make_statement_cppquote(const char *str)
{
    statement_t *stmt = make_statement(STMT_CPPQUOTE);
    stmt->u.str = str;
    return stmt;
}

static statement_t *make_statement_importlib(const char *str)
{
    statement_t *stmt = make_statement(STMT_IMPORTLIB);
    stmt->u.str = str;
    return stmt;
}

static statement_t *make_statement_import(const char *str)
{
    statement_t *stmt = make_statement(STMT_IMPORT);
    stmt->u.str = str;
    return stmt;
}

static statement_t *make_statement_module(type_t *type)
{
    statement_t *stmt = make_statement(STMT_MODULE);
    stmt->u.type = type;
    return stmt;
}

static statement_t *make_statement_typedef(declarator_list_t *decls)
{
    declarator_t *decl, *next;
    statement_t *stmt;
    type_list_t **type_list;

    if (!decls) return NULL;

    stmt = make_statement(STMT_TYPEDEF);
    stmt->u.type_list = NULL;
    type_list = &stmt->u.type_list;

    LIST_FOR_EACH_ENTRY_SAFE( decl, next, decls, declarator_t, entry )
    {
        var_t *var = decl->var;
        type_t *type = find_type_or_error(var->name, 0);
        *type_list = xmalloc(sizeof(type_list_t));
        (*type_list)->type = type;
        (*type_list)->next = NULL;

        type_list = &(*type_list)->next;
        free(decl);
        free(var);
    }

    return stmt;
}

static statement_list_t *append_statements(statement_list_t *l1, statement_list_t *l2)
{
    if (!l2) return l1;
    if (!l1 || l1 == l2) return l2;
    list_move_tail (l1, l2);
    return l1;
}

static attr_list_t *append_attribs(attr_list_t *l1, attr_list_t *l2)
{
    if (!l2) return l1;
    if (!l1 || l1 == l2) return l2;
    list_move_tail (l1, l2);
    return l1;
}

static statement_list_t *append_statement(statement_list_t *list, statement_t *stmt)
{
    if (!stmt) return list;
    if (!list)
    {
        list = xmalloc( sizeof(*list) );
        list_init( list );
    }
    list_add_tail( list, &stmt->entry );
    return list;
}

void init_loc_info(loc_info_t *i)
{
    i->input_name = input_name ? input_name : "stdin";
    i->line_number = line_number;
    i->near_text = parser_text;
}

static void check_def(const type_t *t)
{
    if (t->defined)
        error_loc("%s: redefinition error; original definition was at %s:%d\n",
                  t->name, t->loc_info.input_name, t->loc_info.line_number);
}
