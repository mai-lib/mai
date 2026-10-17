#include "global.h"
#include "ast.h"
#include "grammar.h"
#include <ctype.h>
#include <limits.h>
#include <errno.h>
static inline int l_getc(pctx_t *ctx)
    { return ctx->get(ctx); }
static inline void l_ungetc(int c, pctx_t *ctx)
    { ctx->unget(c, ctx); }
#define CPCHARS(cond)                                   \
    for(; (cond) && c && ptr < end; c = l_getc(ctx))    \
        *((ptr)++) = c
#define CPCHAR(cond)                             \
    if((cond) && c && ptr < end)                 \
    {                                            \
        *((ptr)++) = c;                          \
        c = l_getc(ctx);                         \
    }
static int eat_ws(pctx_t *ctx)
{
    int c = l_getc(ctx);

    while(c == ' ' || c == '\t')
        c = l_getc(ctx);
    if(c == '#')
    {
        c = l_getc(ctx);
        while(c != '\n' && c != '\0' && c != EOF)
            c = l_getc(ctx);
    }
    return c;
}
static int make_integer(YYSTYPE *out, pctx_t *ctx)
{
    long int val = 0;
    char *ptr;
    errno = 0;
    val = strtol(ctx->buf, &ptr, 10);
    if(ptr == ctx->buf)
    {
        yyerror(ctx, "can't parse integer literal");
        return LEXERR;
    }
    else if(errno == ERANGE || val > INT_MAX || val < INT_MIN)
    {
        yyerror(ctx, "integer literal out of range");
        return LEXERR;
    }

    *out = (ast_t) { .type = AST_INT, .integer = (int)val };
    return LIT_INT;
}
static int make_double(YYSTYPE *out, pctx_t *ctx)
{
    double val = 0;
    char *ptr;
    errno = 0;
    val = strtod(ctx->buf, &ptr);
    if(ptr == ctx->buf)
    {
        yyerror(ctx, "can't parse floating-point literal");
        return LEXERR;
    }
    else if(errno == ERANGE)
    {
        yyerror(ctx, "floating-point literal out of range");
        return LEXERR;
    }

    *out = (ast_t) { .type = AST_DOUBLE, .dfloat = val };
    return LIT_DBL;
}
static int lex_number(pctx_t *ctx, int c)
{
    char *ptr = ctx->buf, *end = ctx->buf + L_BUFSIZE - 1;
    int tok = LIT_INT;
    CPCHARS(isdigit(c));
    if(c == '.')
    {
        tok = LIT_DBL;
        CPCHAR(true);
        CPCHARS(isdigit(c));
    }
    if(c == 'e' || c == 'E')
    {
        tok = LIT_DBL;
        CPCHAR(true);
        CPCHAR(c == '+' || c == '-');
        CPCHARS(isdigit(c));
    }
    l_ungetc(c, ctx);
    assert(ptr < end);
    *ptr = '\0';
    return tok;
}
static int make_number(YYSTYPE *out, pctx_t *ctx, int c)
{
    int tok = lex_number(ctx, c);
    if(tok == LIT_DBL)
        return make_double(out, ctx);
    else if(tok == LIT_INT)
        return make_integer(out, ctx);
    return tok;
}
static char *lex_id(pctx_t *ctx, int c)
{
    char *ptr = ctx->buf, *end = ctx->buf + L_BUFSIZE - 1;

    CPCHARS(isalnum(c) || c == '_' || c == '.');
    l_ungetc(c, ctx);
    assert(ptr < end);
    *ptr = '\0';
    return ctx->buf;
}
static bool lex_quoted(pctx_t *ctx, char close)
{
    char *ptr = ctx->buf, *end = ctx->buf + L_BUFSIZE - 1;
    int c = l_getc(ctx);

    while(1)
    {
        CPCHARS(c != close && c != '\\');
        if(c == '\\')
        {
            c = l_getc(ctx);
            if(c=='n')
                c = '\n';
            CPCHAR(true);
            continue;
        }
        break;
    }

    if(c != close)
    {
        l_ungetc(c, ctx);
        return false;
    }
    assert(ptr < end);
    *ptr = '\0';
    return true;
}
typedef struct
{
    const char *str;
    int token;
} kwdspec_t;
static const kwdspec_t op2[] = {
    { "&&", AND },
    { "||", OR },
    { "!=", NE },
    { "==", EQ },
    { ">=", GE },
    { "<=", LE },
};
static const kwdspec_t kwd[] = {
    { "type", TYPE },
    { "if", IF },
    { "else", ELSE },
    { "function", FUNCTION },
    { "return", RET },
    { "while", WHILE },
    { "for", FOR },
    { "break", BREAK },
    { "continue", CONTINUE },
    { "let", LET },
    { "var", VAR },
    { "global", GLOBAL },
    { "const", CONST },
    { "include", INCLUDE },
};
static int match_kwd_char(const kwdspec_t *tab, int len, int c)
{
    for(int i=0; i<len; i++)
        if(tab[i].str[0] == c)
            return i;
    return -1;
}
static int match_kwd_str(const kwdspec_t *tab, int len, char *str)
{
    for(int i=0; i<len; i++)
        if(!strcmp(tab[i].str, str))
            return i;
    return -1;
}
static inline int make_tok(YYSTYPE *out, int tok)
{
    *out = (ast_t) { .type = AST_TOKEN, .token = tok };
    return tok;
}
static const char *find_kwd(const kwdspec_t *tab, int len, int token)
{
    for(int i=0; i<len; i++)
        if(tab[i].token == token)
            return tab[i].str;
    return NULL;
}
const char *ast_str(ast_t *ast)
{
    static char buf[2];

    assert(ast->type == AST_SYMBOL || ast->type == AST_NAME || ast->type == AST_TOKEN);
    if(ast->type == AST_SYMBOL || ast->type == AST_NAME)
        return r_symstr(ast->symbol);

    const char *str = find_kwd(op2, lengthof(op2), ast->token);
    if(str)
        return str;
    str = find_kwd(kwd, lengthof(kwd), ast->token);
    if(str)
        return str;
    buf[0] = ast->token;
    return buf;
}
static inline int make_sym(YYSTYPE *out, char *str, int tok)
{
    asttype type = (tok == ID) ? AST_SYMBOL : AST_QUOTED;
    *out = (ast_t) { .type = type, .symbol = r_intern(str) };
    return tok;
}
static inline int make_str(YYSTYPE *out, char *str, int tok)
{
    *out = (ast_t) { .type = AST_STRING, .string = strdup(str) };
    return tok;
}
static inline int make_delim(YYSTYPE *out, pctx_t *ctx, char delim, int tok,
                             int (*assign)(YYSTYPE *, char *, int))
{
    if(!lex_quoted(ctx, delim))
        return LEXERR;
    return assign(out, ctx->buf, tok);
}
static int l_lex(YYSTYPE *out, pctx_t *ctx)
{
    int c = eat_ws(ctx);
    *out = ast_null;
    switch(c)
    {
    case '\0':
    case EOF:
        return c;
    case '0' ... '9':
        return make_number(out, ctx, c);
    case '\n':
        while((c = eat_ws(ctx)) == '\n');
        l_ungetc(c, ctx);
        return '\n';
    case '`':
        return make_delim(out, ctx, '`', ID, make_sym);
    case '\'':
        return make_delim(out, ctx, '\'', LIT_SYM, make_sym);
    case '\"':
        return make_delim(out, ctx, '"', LIT_STR, make_str);
    default:
        break;
    }
    int i = match_kwd_char(op2, lengthof(op2), c);
    if(i != -1)
    {
        int token = c;
        c = l_getc(ctx);
        if(c == op2[i].str[1])
            token = op2[i].token;
        else
            l_ungetc(c, ctx);
        return make_tok(out, token);
    }
    if(c == '_' || c == '.' || isalpha(c))
    {
        char *str = lex_id(ctx, c);
        int i = match_kwd_str(kwd, lengthof(kwd), str);
        if(i != -1) // keyword
            return make_tok(out, kwd[i].token);
        return make_sym(out, str, ID);
    }
    return make_tok(out, c);
}
int yylex(YYSTYPE *out, pctx_t *ctx)
{
    int tok;
    if(ctx->ltok != tok_empty)
    {
        tok = ctx->ltok;
        *out = ctx->lval;
        ctx->ltok = tok_empty;
        ctx->lval = (ast_t){ 0 };
    }
    else
        tok = l_lex(out, ctx);
    if(ctx->ifexpr)
    {
        if(tok != ELSE)
        {
            ctx->ltok = tok;
            ctx->lval = *out;
            tok = ';';
        }
        ctx->ifexpr = false;
    }
    if(tok == '\n')
    {
        if(!ctx->expr)
        {
            ctx->brk = true;
            return yylex(out, ctx);
        }
    }
    ctx->expr = false;
    return tok;
}
