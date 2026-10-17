#include "global.h"
#include "ast.h"
#include "grammar.h"
#include <signal.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
ast_t ast_null = { .type = AST_INVALID };
static int file_getc(pctx_t *ctx)
    { return getc(ctx->data); }
static void file_ungetc(int c, pctx_t *ctx)
    { ungetc(c, ctx->data); }
typedef struct
{
    char *str, *ptr, *end;
} linedata_t;
static int line_getc(pctx_t *ctx)
{
    linedata_t *data = ctx->data;
    int c = EOF;

    if(data->ptr < data->end)
        c = *data->ptr++;
    if(c == '\0')
        c = '\n';
    return c;
}
static void line_ungetc(int c, pctx_t *ctx)
{
    linedata_t *data = ctx->data;

    if(c != EOF)
        data->ptr--;
}
static void pctx_init(pctx_t *ctx, int (*get)(pctx_t *),
                      void (*unget)(int, pctx_t *), void *data)
{
    *ctx = (pctx_t) {
        .ps = yypstate_new(),
        .result = ast_null,
        .buf = xmalloc(L_BUFSIZE),
        .ltok = tok_empty,
        .get = get,
        .unget = unget,
        .data = data
    };
}
static void pctx_fini(pctx_t *ctx)
{
    yypstate_delete(ctx->ps);
    xfree(ctx->buf);
}
static int read_line(pctx_t *ctx, char *prompt)
{
    linedata_t *data = ctx->data;
    char *line = readline(prompt);

    rl_free_undo_list();
    if(!line)
        return -1;
    if(*line)
        add_history(line);
    *data = (linedata_t) {
        .str = line,
        .ptr = line,
        .end = line + strlen(line) + 1
    };
    return 0;
}
static int parse_line(pctx_t *ctx, bool isfirst)
{
    linedata_t *data = ctx->data;
    int status = isfirst ? 1 : YYPUSH_MORE;

    ctx->brk = false;
    do
    {
        ast_t val;
        int tok = yylex(&val, ctx);
        if(!ctx->brk)
            status = yypush_parse(ctx->ps, tok, &val, ctx);
    } while(!ctx->brk && status == YYPUSH_MORE);
    xfree(data->str);
    return status;
}
int p_readline(char *p1, char *p2, ast_t *result)
{
    linedata_t data;
    pctx_t ctx;
    int status;
    char *prompt = p1;

    pctx_init(&ctx, line_getc, line_ungetc, &data);
    do
    {
        status = read_line(&ctx, prompt);
        if(status != 0)
            break;
        status = parse_line(&ctx, prompt == p1);
        prompt = p2;
    } while(status == YYPUSH_MORE);
    if(status == 0)
        *result = ctx.result;
    else
        ast_fini(&ctx.result);
    pctx_fini(&ctx);
    return status;
}
static int p_file(FILE *input, ast_t *result)
{
    int status;
    pctx_t ctx;

    pctx_init(&ctx, file_getc, file_ungetc, input);
    ctx.nest = 1;
    status = yypull_parse(ctx.ps, &ctx);
    if(status == 0)
        *result = ctx.result;
    else
        ast_fini(&ctx.result);
    pctx_fini(&ctx);
    return status;
}
int p_source(char *name, ast_t *result)
{
    int r;
    FILE *fp = fopen(name, "r");

    if(fp)
    {
        r = p_file(fp, result);
        fclose(fp);
    }
    else
        r = -errno;
    return r;
}
void yyerror(pctx_t *ctx, const char *err)
{
    c_error("%s", err);
}
