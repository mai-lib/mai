#include "global.h"
#include "ir.h"
#include "opt.h"
typedef struct
{
    call_ctx_t base;
    cnode_array_t args;
    cresult *pres;
} ir_call_ctx_t;
static bool set_arg(void *ptr, argdesc_t *arg, void *val)
{
    ir_call_ctx_t *ctx = ptr;
    int i = arg - ctx->base.sig->args;

    aset(&ctx->args, i, val);
    return true;
}
static bool append_rest(void *ptr, rsymbol_t *name, void *val)
    { return false; }
static bool fail_rest(void *ptr, rsymbol_t *name, void *val)
{
    ir_call_ctx_t *ctx = ptr;

    if(name)
        c_error("unknown argument `%s`.", r_symstr(name));
    else
        c_error("too many arguments.");
    *ctx->pres = FAILED;
    return false;
}
static void call_become_fast(cnode_t *node, funsig_t *sig,
                             cnode_array_t *args, argbits_t argbits)
{
    array_fini(&node->call.args);
    array_fini(&node->call.names);
    node->type = CN_CALL_FAST;
    node->call.args = *args;
    node->call.argbits = argbits;
}
cresult call_normalise(cnode_t *node, funsig_t *sig)
{
    cresult res = SUCCESS;
    cnode_t **args = adata(&node->call.args);
    rsymbol_t **names = call_has_names(node)
                      ? adata(&node->call.names) : NULL;
    int nargs = alen(&node->call.args);
    ir_call_ctx_t ctx = {
        .base = {
            .posbits = 0,
            .argbits = 0,
            .sig = sig,
            .append_rest = sig->has_rest
                         ? append_rest : fail_rest,
            .set_arg = set_arg
        },
        .args = ARRAY_INIT,
        .pres = &res
    };

    array_resize(&ctx.args, sig->nargs);
    if(call_sequence(&ctx, (void **)args, names, nargs, no_match_rest,
                     call_match_kwd, call_match_pos, call_match_omit))
    {
        if(sig->reqbits != (ctx.base.argbits & sig->reqbits))
        {
            res |= FAILED;
            c_error("required argument not supplied.");
        }
        else
        {
            res |= CHANGED;
            call_become_fast(node, sig, &ctx.args, ctx.base.argbits);
        }
    }
    else
        array_fini(&ctx.args);
    return res;
}
