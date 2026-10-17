%code top {
  #include "global.h"
  #include "ast.h"
  #include "grammar.h"
}
%code requires {
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
}
%code {
  static ast_t ast_va(ast_t *head, ...);
  static ast_t ast_set(ast_t *expr, ast_t *ast, int i);
  static ast_t ast_append(ast_t *expr, ast_t *ast);
  static ast_t ast_prepend(ast_t *expr, ast_t *ast);
  #define AST1(a) ast_va(&a, NULL)
  #define AST2(a, b) ast_va(&a, &b, NULL)
  #define AST3(a, b, c) ast_va(&a, &b, &c, NULL)
  #define AST4(a, b, c, d) ast_va(&a, &b, &c, &d, NULL)
  #define ENDN(l, a, i) ast_set(&l, &a, i)
  #define END1(l, a) ENDN(l, a, 0)
  #define PUSH(l, a) ast_append(&l, &a)
  #define PREP(l, a) ast_prepend(&l, &a)
}
%define api.pure
%define api.push-pull both
%parse-param {pctx_t *ctx}
%lex-param {pctx_t *ctx}
%destructor { if(!ctx->done) ast_fini(&$$); } <>
%error-verbose // deprecated; should be %define parse.error verbose in 3.0+
%token LIT_INT LIT_DBL LIT_BOOL LIT_SYM LIT_STR ID TYPE FUNCTION
%token WHILE FOR BREAK CONTINUE LET VAR GLOBAL CONST INCLUDE LEXERR
%precedence RET
%precedence IF
%precedence ELSE
%right '='
%left OR AND
%nonassoc '>' GE '<' LE EQ NE
%left '&' '|'
%left '+' '-'
%left '*' '/' '%'
%left ':'
%precedence '!' UMINUS UPLUS
%left '^'
%precedence '[' '('
%%
start:      exprlist { ctx->done = true; ctx->result = $1; }
    |       error { YYABORT; } ;
exprlist:   dexpr { $$ = AST1($1); }
    |       exprlist eos dexpr { $$ = PUSH($1, $3); }
    |       exprlist eos { $$ = $1; } ;
eos:        '\n' { if(ctx->nest > 0) ctx->brk = true; }
    |       ';' ;
nl:         { $$ = ast_null; } | '\n' { ctx->brk = true; } ;
paren:      '(' { ctx->nest++; } ;
bracket:    '[' { ctx->nest++; } ;
brace:      '{' { ctx->nest++; } ;
parclo:     nl ')' { ctx->nest--; } ;
brkclo:     nl ']' { ctx->nest--; } ;
braclo:     '}' { ctx->nest--; } ;
dexpr:      aexpr
    |       LET decllist { $$ = AST2($1, $2); }
    |       VAR decllist { $$ = AST2($1, $2); }
    |       GLOBAL decllist { $$ = AST2($1, $2); }
    |       CONST decllist { $$ = AST2($1, $2); } ;
aexpr:      expr %prec '=' { $$ = $1; }
    |       expr '=' aexpr { $$ = AST3($2, $1, $3); } ;
pexpr:      paren aexpr parclo { $$ = $2; } ;
expr:       primary
            {
                $$ = $1;
                if(yychar == YYEMPTY || yychar == '\n')
                    ctx->expr = true;
            }
    |       ifexpr { $$ = $1; ctx->expr = true; }
    |       ifexpr elseexpr { $$ = PUSH($1, $2); ctx->expr = true; } ;
ifexpr:     IF pexpr aexpr
            {
                $$ = AST3($1, $2, $3);
                if(yychar == '\n' && ctx->nest > 0)
                {
                    ctx->ifexpr = true;
                    yyclearin;
                    ctx->brk = true;
                }
            } ;
elseexpr:   ELSE aexpr { $$ = $2; } ;
primary:    ID
    |       literal
    |       pexpr
    |       block
    |       unary
    |       binary
    |       expr args { $$ = END1($2, $1); }
    |       expr subs { $$ = ENDN($2, $1, 1); }
    |       FUNCTION formals rtopt nl aexpr { $$ = AST4($1, $3, $2, $5); }
    |       WHILE pexpr aexpr { $$ = AST3($1, $2, $3); }
    |       FOR forlist aexpr { PUSH($2, $3); $$ = END1($2, $1); }
    |       RET expr { $$ = AST2($1, $2); }
    |       BREAK
    |       CONTINUE
    |       INCLUDE LIT_STR { $$ = AST2($1, $2); } ;
literal:    LIT_INT | LIT_DBL | LIT_BOOL | LIT_STR | LIT_SYM
    |       TYPE paren type parclo { $$ = AST2($1, $3); } ;
block:      brace exprlist braclo { $$ = PREP($2, $1); }
    |       brace braclo { $$ = AST1($1); } ;
unary:      '!' expr
            {
                #define UNOP $$ = AST2($1, $2)
                UNOP;
            }
    |       '-' expr %prec UMINUS { UNOP; }
    |       '+' expr %prec UPLUS  { UNOP; }
binary:     expr ':' expr
            {
                #define BINOP $$ = AST3($2, $1, $3)
                BINOP;
            }
    |       expr '+' expr { BINOP; }
    |       expr '-' expr { BINOP; }
    |       expr '*' expr { BINOP; }
    |       expr '/' expr { BINOP; }
    |       expr '^' expr { BINOP; }
    |       expr '%' expr { BINOP; }
    |       expr '&' expr { BINOP; }
    |       expr '|' expr { BINOP; }
    |       expr '<' expr { BINOP; }
    |       expr LE expr  { BINOP; }
    |       expr EQ expr  { BINOP; }
    |       expr NE expr  { BINOP; }
    |       expr GE expr  { BINOP; }
    |       expr '>' expr { BINOP; }
    |       expr AND expr { BINOP; }
    |       expr OR expr  { BINOP; } ;
rtopt:      { $$ = ast_null; } | ':' type { $$ = $2; } ;
formals:    paren decllist parclo { $$ = $2; }
    |       paren parclo { $$ = ast_va(NULL); } ;
forlist: paren dlopt ';' aexpr ';' aexopt parclo
    { $$ = AST4(ast_null, $2, $4, $6); } ;
dlopt:      { $$ = ast_null; }
    |       decllist ;
aexopt:     { $$ = ast_null; }
    |       aexpr ;
args:       paren arglist parclo { $$ = $2; }
    |       paren ')' { $$ = AST1(ast_null); ctx->nest--; } ;
subs:       bracket arglist brkclo { $$ = PREP($2, $1); }
    |       bracket ']' { $$ = AST2($1, ast_null); ctx->nest--; } ;
arglist:    arg { $$ = AST2(ast_null, $1); }
    |       name '=' expr { $$ = AST3(ast_null, $1, $3); }
    |       arglist ',' arg { $$ = PUSH($1, $3); }
    |       arglist ',' name '=' expr { PUSH($1, $3); $$ = PUSH($1, $5); } ;
arg:        { $$ = ast_null; } | expr ;
name:       ID { $$ = $1; $$.type = AST_NAME; ctx->expr = true; } ;
decllist:   decl { $$ = AST1($1); }
    |       decllist ',' decl { $$ = PUSH($1, $3); } ;
decl:       name
    |       name initexpr { $$ = AST2($1, $2); }
    |       type name { $$ = AST2($1, $2); }
    |       type name initexpr { $$ = AST3($1, $2, $3); } ;
initexpr:   '=' expr { $$ = $2; } ;
type:       name
    |       name typeargs { $$ = END1($2, $1); }
    |       TYPE
    |       FUNCTION
    |       FUNCTION typeargs { $$ = PREP($2, $1); }
    |       FUNCTION typeargs ':' type { END1($2, $4); $$ = PREP($2, $1); } ;
typeargs:   paren typelist parclo { $$ = $2; }
    |       paren parclo { $$ = AST1(ast_null); } ;
typelist:   type { $$ = AST2(ast_null, $1); }
    |       typelist ',' type { $$ = PUSH($1, $3); } ;
%%
const int tok_empty = YYEMPTY;
static ast_t ast_va(ast_t *head, ...)
{
    ast_array_t *arr;

    if(head)
    {
        va_list ap;
        ast_t *ptr;

        array_alloc(&arr, 2);
        array_push(arr, *head);
        va_start(ap, head);
        while((ptr = va_arg(ap, ast_t *)))
            array_push(arr, *ptr);
        va_end(ap);
    }
    else
        array_alloc(&arr, 0);
    return (ast_t) { .type = AST_NODE, .children = arr };
}
static ast_t ast_set(ast_t *expr, ast_t *ast, int i)
{
    assert(expr->type == AST_NODE && expr->children);
    assert(alen(expr->children) > i);
    aset(expr->children, i, *ast);
    return *expr;
}
static ast_t ast_append(ast_t *expr, ast_t *ast)
{
    assert(expr->type == AST_NODE && expr->children);
    array_push(expr->children, *ast);
    return *expr;
}
static ast_t ast_prepend(ast_t *expr, ast_t *ast)
{
    assert(expr->type == AST_NODE && expr->children);
    array_insert(expr->children, 0, *ast);
    return *expr;
}
static void ast_free_node(ast_array_t *arr)
{
    ast_t *ptr;

    array_foreach_ptr(arr, ptr)
        ast_fini(ptr);
    array_free(arr);
}
void ast_fini(ast_t *ast)
{
    if(ast->type == AST_NODE)
        ast_free_node(ast->children);
    else if(ast->type == AST_STRING)
        xfree(ast->string);
}
