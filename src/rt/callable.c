#include "global.h"
// required for rbuiltin_define
#include "rt/builtin.h"
// required for from_spec
#include "ast.h"
static void closure_gc(rclosure_t *cl);
static void rcall_gc(void *ptr)
{
    if(!rcall_is_builtin(ptr))
        closure_gc(ptr);
}
static void closure_free(rclosure_t *cl);
static void rcall_free(void *ptr)
{
    if(!rcall_is_builtin(ptr))
        closure_free(ptr);
}
static void rcall_print(FILE *fp, const void *ptr)
{
    const rcallable_t *cl = ptr;

    fprintf(fp, "<");
    r_print(fp, r_typeof(ptr));
    if(rcall_is_builtin(cl))
    {
        const cbuiltin_t *cbi = ((rbuiltin_t *)ptr)->cbi;

        if(cbi)
        {
            fprintf(fp, " `%s`>", cbi->name);
            return;
        }
    }
    fprintf(fp, " %p>", ptr);
}
static const typeops_t call_ops = {
    .free = rcall_free,
    .gc = rcall_gc,
    .print = rcall_print
};
bool sig_equal(funsig_t *a, funsig_t *b)
{
    if(a->nargs != b->nargs || a->ret_type != b->ret_type)
        return false;
    for(int i=0; i<a->nargs; i++)
    {
        argdesc_t *m = &a->args[i], *n = &b->args[i];

        if(m->name != n->name ||
           m->is_optional != n->is_optional ||
           m->type != n->type ||
           m->offset != n->offset)
            return false;
    }
    return true;
}
bool sig_types_equal(funsig_t *a, funsig_t *b)
{
    if(a->nargs != b->nargs || a->ret_type != b->ret_type)
        return false;
    for(int i=0; i<a->nargs; i++)
    {
        argdesc_t *m = &a->args[i], *n = &b->args[i];

        if(m->type != n->type)
            return false;
    }
    return true;
}
void sig_print(FILE *fp, funsig_t *sig)
{
    fprintf(fp, "function ");
    fputc('(', fp);
    for(int i=0; i<sig->nargs; i++)
    {
        argdesc_t *arg = &sig->args[i];

        if(i > 0)
            fprintf(fp, ", ");
        if(arg->name == r_sym_rest)
            r_print(fp, arg->name);
        else if(arg->name)
        {
            if(arg->type != r_type_object)
            {
                r_print(fp, arg->type);
                fputc(' ', fp);
            }
            r_print(fp, arg->name);
            if(arg->is_optional)
                fputc('=', fp);
        }
        else
            r_print(fp, arg->type);
    }
    fputc(')', fp);
    if(sig->ret_type != r_type_object)
    {
        fprintf(fp, " : ");
        r_print(fp, sig->ret_type);
    }
}
static inline void argdesc_mark(argdesc_t *arg)
{
    gc_mark(arg->name);
    gc_mark(arg->type);
}
void sig_gc(funsig_t *sig)
{
    gc_mark(sig->ret_type);
    for(int i=0; i<sig->nargs; i++)
        argdesc_mark(&sig->args[i]);
}
void sig_free(funsig_t *sig)
{
    if(sig->args)
        xfree(sig->args);
    xfree(sig);
}
static uint32_t copy_args(funsig_t *sig, unsigned nargs, argdesc_t *args)
{
    uint32_t hash = 0;

    for(int i=0; i<nargs; i++)
    {
        argdesc_t *arg = &sig->args[i];

        *arg = args[i];
        arg->offset = sig->argsz;
        if(arg->name == r_sym_rest)
            sig->has_rest = true;
        if(!arg->is_optional)
            sig->reqbits |= 1<<i;
        sig->argsz += rtype_eltsz(arg->type);
        hash = hash_code_seed(&arg->is_optional, sizeof(bool), hash);
        hash = hash_roll(hash, r_hash(arg->name));
        hash = hash_roll(hash, r_hash(arg->type));
    }
    return hash;
}
rtype_t *rcall_type_create(rtype_t *ret_type, unsigned nargs,
                           argdesc_t *args)
{
    funsig_t *sig = xmalloc(sizeof(funsig_t));

    *sig = (funsig_t) {
        .args = nargs > 0 ? xcalloc(nargs, sizeof(argdesc_t)) : NULL,
        .nargs = nargs,
        .reqbits = 0,
        .argsz = sizeof(argbits_t),
        .ret_type = ret_type,
        .has_rest = false
    };
    sig->hash = hash_roll(r_hash(ret_type), copy_args(sig, nargs, args));
    return rtype_create(RT_CALLABLE, sig, &call_ops, NULL);
}
static rtype_t *
call_type_from_spec(const consdesc_t *cons, unsigned nargs, ast_t *args)
{
    if(nargs < 1)
        return r_type_callable;

    argdesc_t *descs = alloca(sizeof(argdesc_t) * (nargs-1));
    rtype_t *ret_type = (args[0].type == AST_INVALID)
        ? r_type_object
        : rtype_from_spec(&args[0]);

    if(!ret_type)
        return NULL;
    for(int i=1; i<nargs; i++)
    {
        argdesc_t *desc = &descs[i-1];
        ast_t *arg = &args[i];

        if(arg->type == AST_NAME && arg->symbol == r_sym_rest)
        {
            if(i != nargs-1)
            {
                c_warning("`...` not last argument in list.");
                return NULL;
            }
            *desc = argdesc_init(r_sym_rest, (rtype_t *)r_type_vec_object, true);
        }
        else
        {
            rtype_t *type = rtype_from_spec(arg);

            if(!type)
                return NULL;
            *desc = argdesc_init(NULL, type, false);
        }
    }
    return rcall_type_create(ret_type, nargs-1, descs);
}
static consdesc_t callable_cons = {
    .from_spec = call_type_from_spec,
    .kind = RT_CALLABLE
};
static void closure_free(rclosure_t *cl)
{
    if(cl->env)
        xfree(cl->env);
}
#define env_ptr(c, f) (robject_t **)((uint8_t *)c->env + c->fn->f)
static void closure_gc(rclosure_t *cl)
{
    gc_mark(cl->fn);
    if(cl->env)
    {
        robject_t **ptr;

        for(ptr = env_ptr(cl, env_scalsz);
            ptr < env_ptr(cl, env_sz); ptr++)
            gc_mark(*ptr);
    }
}
rclosure_t *rcall_closure_create(rtype_t *type, rfunction_t *fn)
{
    rclosure_t *cl = gc_alloc(type, sizeof(rclosure_t));
    op_offset_t env_sz = fn->env_sz;

    cl->base.disp = CALL_CLOSURE;
    cl->fn = fn;
    cl->env = (env_sz > 0) ? xcalloc(1, env_sz) : NULL;
    return cl;
}
rfunction_t *rfunc_create(rtype_t *cl_type, op_code_t *code, size_t loc_scalsz,
                          size_t loc_sz, size_t env_scalsz, size_t env_sz,
                          robject_array_t *consts)
{
    rfunction_t *rfn = gc_alloc(r_type_function, sizeof(rfunction_t));
    unsigned nconsts = alen(consts);

    *rfn = (rfunction_t) {
        .base = rfn->base,
        .code = code,
        .cl_type = cl_type,
        .loc_scalsz = loc_scalsz,
        .loc_sz = loc_sz,
        .env_scalsz = env_scalsz,
        .env_sz = env_sz,
        .consts = array_cede(consts),
        .nconsts = nconsts
    };
    return rfn;
}
static void rfunc_free(void *ptr)
{
    rfunction_t *fn = ptr;
    if(fn->consts)
        xfree(fn->consts);
    xfree(fn->code);
}
static void rfunc_gc(void *ptr)
{
    rfunction_t *fn = ptr;
    gc_mark(fn->cl_type);
    for(int i=0; i<fn->nconsts; i++)
        gc_mark(fn->consts[i]);
}
static const typeops_t func_ops = {
    .free = rfunc_free,
    .gc = rfunc_gc,
};
rcallable_t *
rcall_builtin_create(rtype_t *type, builtin_fn fn, const cbuiltin_t *cbi)
{
    rbuiltin_t *cl = gc_alloc(type, sizeof(rbuiltin_t));

    cl->base.disp = CALL_BUILTIN;
    cl->fn = fn;
    cl->cbi = cbi;
    return (rcallable_t *)cl;
}
static inline unsigned count_init_args(const builtin_init_t *init)
{
    unsigned i = 0;
    for(; init->args[i].ptype; i++);
    return i;
}
static inline argdesc_t argdesc_from_init(const builtin_init_arg_t *arg)
{
    return argdesc_init(arg->name ? r_intern(arg->name) : NULL,
                        *arg->ptype,
                        arg->is_optional);
}
rcallable_t *rbuiltin_define(const builtin_init_t *init)
{
    unsigned nargs = count_init_args(init);
    argdesc_t *args = alloca(sizeof(argdesc_t) * nargs);

    for(int i=0; i<nargs; i++)
        args[i] = argdesc_from_init(&init->args[i]);

    rtype_t *cl_type = rcall_type_create(*init->prtype, nargs, args);
    rcallable_t *cl = rcall_builtin_create(cl_type, init->fn, init->cbi);

    r_defbuiltin(init->name, cl);
    return cl;
}
void rbuiltin_install(const builtin_init_t *init)
{
    for(; init->name; init++)
        rbuiltin_define(init);
}
rtype_t *r_type_function, *r_type_callable;
void rt_install_call_types()
{
    r_type_function = rtype_create(RT_OBJECT, NULL, &func_ops, "compiled");
    r_type_callable = rtype_cons_init(&callable_cons, "function");
}
