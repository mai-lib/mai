#include "global.h"
bool call_sequence(void *ptr, void **args, rsymbol_t **names, int nargs,
                   bool (*rest_fn)(void *, void *),
                   bool (*kwd_fn)(void *, rsymbol_t *, void *),
                   bool (*pos_fn)(void *, void *),
                   bool (*omit_fn)(void *))
{
    bool r = true;

    if(names)
    {
        for(int i = 0; r && i < nargs; i++)
        {
            void *arg = args[i];
            rsymbol_t *name = names[i];

            if(name && name != r_sym_rest)
                r &= kwd_fn(ptr, name, arg);
        }
    }
    for(int i = 0; r && i < nargs; i++)
    {
        void *arg = args[i];

        if(names && names[i] == r_sym_rest)
            r &= rest_fn(ptr, arg);
        else if(names && names[i])
            continue;
        else if(arg)
            r &= pos_fn(ptr, arg);
        else
            r &= omit_fn(ptr);
    }
    return r;
}
static inline int nargs_excluding_rest(funsig_t *sig)
{
    return sig->has_rest ? sig->nargs - 1 : sig->nargs;
}
static argdesc_t *get_kwd_arg(funsig_t *sig, rsymbol_t *name, int *iptr)
{
    for(int i = 0; i < sig->nargs; i++)
    {
        argdesc_t *arg = &sig->args[i];

        if(arg->name == name)
        {
            *iptr = i;
            return arg;
        }
    }
    return NULL;
}
static argdesc_t *get_pos_arg(funsig_t *sig, int i)
{
    if(i < 0 || i >= nargs_excluding_rest(sig))
        return NULL;
    return &sig->args[i];
}
static inline bool set_argbit(call_ctx_t *ctx, int i, bool omit)
{
    if(i >= sizeof(argbits_t)*8)
        return false;
    if(!omit)
        ctx->argbits |= 1<<i;
    ctx->posbits |= 1<<i;
    return true;
}
bool call_match_kwd(void *ptr, rsymbol_t *name, void *val)
{
    call_ctx_t *ctx = ptr;
    int i;
    argdesc_t *arg = get_kwd_arg(ctx->sig, name, &i);

    if(!arg)
        return ctx->append_rest(ctx, name, val);
    return set_argbit(ctx, i, false) && ctx->set_arg(ctx, arg, val);
}
bool call_match_pos(void *ptr, void *val)
{
    call_ctx_t *ctx = ptr;
    int i = ffs(~ctx->posbits)-1;
    argdesc_t *arg = get_pos_arg(ctx->sig, i);

    if(!arg)
        return ctx->append_rest(ctx, NULL, val);
    return set_argbit(ctx, i, false) && ctx->set_arg(ctx, arg, val);
}
bool call_match_omit(void *ptr)
{
    call_ctx_t *ctx = ptr;
    int i = ffs(~ctx->posbits)-1;
    bool r = true;

    if(i < 0)
        return false;
    if(i >= nargs_excluding_rest(ctx->sig))
        r = ctx->append_rest(ctx, NULL, NULL);
    return r & set_argbit(ctx, i, true);
}
bool no_match_rest(void *ptr, void *rest)
    { return false; }
bool call_match_rest(void *ptr, void **args, rsymbol_t **names, int nargs)
{
    return call_sequence(ptr, args, names, nargs, no_match_rest,
                         call_match_kwd, call_match_pos, call_match_omit);
}
