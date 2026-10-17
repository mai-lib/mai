#include "global.h"
#include "ir.h"
#include "ast.h"
#include <errno.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>
uint8_t stack[65536];
static int execute(rclosure_t *cl, void **pres)
{
    vm_ctx_t ctx;

    vm_init_ctx(&ctx, stack, sizeof(stack));
    gc_set_enabled(true);
    if(vm_execute(&ctx, cl) == 0 && pres)
        *pres = ctx.ret_val;
    vm_fini_ctx(&ctx);
    gc_set_enabled(false);
    if(ctx.err_msg)
    {
        if(pres)
            *pres = ctx.err_msg;
        else
            xfree(ctx.err_msg);
        return -1;
    }
    return 0;
}
static void evaluate(rclosure_t *cl, bool print)
{
    void *res = NULL;

    if(!cl)
        return;
    if(execute(cl, &res))
    {
        putchar('\n');
        printf("%s", (char *)res);
        xfree(res);
    }
    else if(print)
    {
        putchar('\n');
        r_print(stdout, res);
    }
    putchar('\n');
}
rclosure_t *source(char *filename)
{
    int r;
    rclosure_t *cl = NULL;
    ast_t ast;

    c_begin();
    r = p_source(filename, &ast);
    if(r == 0)
        cl = compile(&ast, filename);
    else if(r < 0)
        c_error("sourcing %s - %s", filename, strerror(errno));
    c_end();
    return cl;
}
static void load(char *filename, bool print)
{
    evaluate(source(filename), print);
}
typedef struct
{
    const char *text;
    int len;
    ARRAY(char *) strs;
} compl_ctx_t;
static void completions(const void *key, void *value, void *ptr)
{
    compl_ctx_t *ctx = ptr;
    const rsymbol_t *name = key;
    char *str = name->string;

    if(!strncmp(str, ctx->text, ctx->len))
        array_push(&ctx->strs, str);
}
static char *complete_global(const char *text, int state)
{
    static int cur;
    static compl_ctx_t ctx;

    if(!state)
    {
        ctx = (compl_ctx_t) { text, strlen(text) };
        array_init(&ctx.strs, 1);
        hashmap_map(r_globals, completions, &ctx);
        cur = 0;
    }
    else
        cur++;
    if(cur >= alen(&ctx.strs))
    {
        array_fini(&ctx.strs);
        return NULL;
    }
    rl_completion_suppress_append = true;
    return strdup(aref(&ctx.strs, cur));
}
#define HISTORY_MAX 500
void readline_init()
{
    rl_initialize();
    rl_completion_entry_function = complete_global;
    rl_completer_quote_characters = "'`";
    rl_completer_word_break_characters = " \t\n;(,)+-*/:<>={}[]";
    using_history();
    stifle_history(HISTORY_MAX);
}
static const char *initname = ".maininit";
static inline char *init_default()
{
    static char buf[PATH_MAX];
    char *base = getenv("HOME");

    if(!base)
        base = ".";
    snprintf(buf, PATH_MAX, "%s/%s", base, initname);
    return buf;
}
static const char *initvar = "INITFILE";
static char *init_name()
{
    char *name = getenv(initvar);
    return name ? name : init_default();
}
static void load_init()
{
    char *name = init_name();
    struct stat sbuf;

    if(stat(name, &sbuf) != 0)
        return;
    load(name, false);
}
static void repl()
{
    ast_t ast;
    int status;

    load_init();
    readline_init();
    c_begin();
    while((status = p_readline("> ", "+ ", &ast)) != -1)
    {
        rclosure_t *cl = NULL;
        if(status == 0)
            cl = compile(&ast, NULL);
        c_end();
        evaluate(cl, true);
        c_begin();
    }
    c_end();
    putchar('\n');
}
static void usage(char *name)
{
    fprintf(stderr, "usage:\t%s [file]\n", name);
    fprintf(stderr, "\tif file is not specified, INITFILE (default %s) is\n"
            "\tloaded if it exists, and the REPL is entered.\n", init_name());
    exit(-1);
}
int main(int argc, char *argv[])
{
    runtime_init();
    if(argc == 1)
        repl();
    else if(argc == 2 && argv[1][0] != '-')
        load(argv[1], true);
    else
        usage(argv[0]);
    runtime_fini();
    return 0;
}
