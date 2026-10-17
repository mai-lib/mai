#include "global.h"
#include "ir.h"
#include "opt.h"
typedef struct copy_ctx
{
    struct copy_ctx *parent;
    cfunction_t *src;
    cfunction_t **functions;
    cblock_t **blocks;
    cnode_t **nodes;
    cvar_t **vars;
} copy_ctx_t;
static cblock_t *copy_into(cfunction_t *, cfunction_t *,
                           copy_ctx_t *, cnode_array_t *);
static cfunction_t *copy_one_function(copy_ctx_t *ctx, cfunction_t *dest,
                                      cfunction_t *child)
{
    cfunction_t *copy = cfunc_create(dest);

    copy->cl_type = child->cl_type;
    copy->nfuncs = child->nfuncs;
    copy->nblocks = child->nblocks;
    copy->nnodes = child->nnodes;
    copy->id = child->id;
    copy->ninlined = child->ninlined;
    copy->entry = copy_into(copy, child, ctx, NULL);
    return copy;
}
static void copy_functions(copy_ctx_t *ctx, cfunction_t *dest)
{
    cfunction_t *src = ctx->src, *fn;

    if(src->nfuncs == 0)
        return;
    ctx->functions = xmalloc(src->nfuncs * sizeof(cfunction_t *));
    list_foreach_entry(&src->cfunc_head, fn, cfunc_list)
        ctx->functions[fn->id] = copy_one_function(ctx, dest, fn);
}
static cvar_t *copy_one_var(cfunction_t *dest, cvar_t *var)
{
    assert(!cvar_is_global(var));
    cvar_t *copy = cvar_create(var->name, var->type, var->decl);

    copy->local = var->local;
    copy->local.binder = dest;
    copy->is_const = var->is_const;
    copy->id = var->id;
    list_add_before(&dest->cvar_head, &copy->cvar_list);
    return copy;
}
static int number_vars(cfunction_t *fn)
{
    int nvars = 0;
    cvar_t *var;

    list_foreach_entry(&fn->cvar_head, var, cvar_list)
        var->id = nvars++;
    return nvars;
}
static void copy_vars(copy_ctx_t *ctx, cfunction_t *dest)
{
    cfunction_t *src = ctx->src;
    int nvars = number_vars(src);
    cvar_t *var;

    if(nvars == 0)
        return;
    ctx->vars = xmalloc(nvars * sizeof(cvar_t *));
    list_foreach_entry(&src->cvar_head, var, cvar_list)
        ctx->vars[var->id] = copy_one_var(dest, var);
}
static cblock_t *copy_one_block(cfunction_t *dest, cblock_t *block)
{
    cblock_t *copy = cblock_create();

    copy->id = block->id;
    copy->start = block->start;
    copy->end = block->end;
    list_add_before(&dest->cblock_head, &copy->cblock_list);
    return copy;
}
static cnode_t *copy_one_node(cblock_t *block, cnode_t *node)
{
    cnode_t *copy = cnode_append(block, node->type);

    copy->id = node->id;
    copy->decl = node->decl;
    copy->file = node->file;
    return copy;
}
static cblock_t *copy_body(copy_ctx_t *ctx, cfunction_t *dest)
{
    cfunction_t *src = ctx->src;
    cblock_t *block, *entry = NULL;

    assert(src->nblocks>0 && src->nnodes>0);
    ctx->blocks = xmalloc(src->nblocks * sizeof(cblock_t *));
    ctx->nodes = xmalloc(src->nnodes * sizeof(cnode_t *));
    list_foreach_entry(&src->cblock_head, block, cblock_list)
    {
        cnode_t *node;
        cblock_t *newblock = copy_one_block(dest, block);

        ctx->blocks[block->id] = newblock;
        if(block == src->entry)
            entry = newblock;
        list_foreach_entry(&block->cnode_head, node, cnode_list)
        {
            cnode_t *newnode = copy_one_node(newblock, node);

            ctx->nodes[node->id] = newnode;
        }
    }
    return entry;
}
static void fixup(copy_ctx_t *ctx, cfunction_t *dest, cnode_array_t *exits);
static cblock_t *copy_into(cfunction_t *dest, cfunction_t *src,
                           copy_ctx_t *parent, cnode_array_t *exits)
{
    cblock_t *entry;
    copy_ctx_t ctx = { .src = src, .parent = parent };

    copy_vars(&ctx, dest);
    copy_functions(&ctx, dest);
    entry = copy_body(&ctx, dest);
    fixup(&ctx, dest, exits);
    if(ctx.vars)
        xfree(ctx.vars);
    if(ctx.nodes)
        xfree(ctx.nodes);
    if(ctx.blocks)
        xfree(ctx.blocks);
    if(ctx.functions)
        xfree(ctx.functions);
    return entry;
}
static cblock_array_t blocks_fixup(copy_ctx_t *ctx, cblock_array_t *psrc)
{
    cblock_array_t dest = ARRAY_INIT;
    cblock_t **ptr;

    array_copy(&dest, psrc);
    array_foreach_ptr(&dest, ptr)
        *ptr = ctx->blocks[(*ptr)->id];
    return dest;
}
static cnode_array_t nodes_fixup(copy_ctx_t *ctx, cnode_array_t *psrc)
{
    cnode_array_t dest = ARRAY_INIT;
    cnode_t **ptr;

    array_copy(&dest, psrc);
    array_foreach_ptr(&dest, ptr)
    {
        if(*ptr)
            *ptr = ctx->nodes[(*ptr)->id];
    }
    return dest;
}
static cvar_t *var_fixup(copy_ctx_t *ctx, cvar_t *var)
{
    if(cvar_is_global(var))
        return var;
    for(; ctx && var->local.binder != ctx->src; ctx = ctx->parent);
    if(!ctx)
        return var;
    return ctx->vars[var->id];
}
#define NODE(f) dnode->f = ctx->nodes[snode->f->id]
#define FUNC(f) dnode->f = ctx->functions[snode->f->id]
#define BLOCKS(f) dblock->f = blocks_fixup(ctx, &sblock->f)
#define NODES(f) dnode->f = nodes_fixup(ctx, &snode->f)
#define VAR(f) dnode->f = var_fixup(ctx, snode->f)
#define COPY(f) dnode->f = snode->f
static void fixup(copy_ctx_t *ctx, cfunction_t *dest,
                  cnode_array_t *exits)
{
    cfunction_t *src = ctx->src;
    cblock_t *sblock;
    cvar_t *var;

    if(exits)
    {
        list_foreach_entry(&src->cvar_head, var, cvar_list)
            if(var->local.is_arg)
                ctx->vars[var->id]->local.is_arg = false;
    }
    else
    {
        array_foreach_entry(&src->closure, var)
            array_push(&dest->closure, var_fixup(ctx, var));
    }
    list_foreach_entry(&src->cblock_head, sblock, cblock_list)
    {
        cblock_t *dblock = ctx->blocks[sblock->id];
        cnode_t *snode;

        BLOCKS(pred);
        BLOCKS(succ);
        list_foreach_entry(&sblock->cnode_head, snode, cnode_list)
        {
            cnode_t *dnode = ctx->nodes[snode->id];

            NODES(users);
            switch(snode->type)
            {
                case CN_CALL:
                case CN_CALL_FAST:
                    NODE(call.target);
                    NODES(call.args);
                    if(snode->type == CN_CALL)
                        array_copy(&dnode->call.names, &snode->call.names);
                    else
                        COPY(call.argbits);
                    break;
                case CN_SET:
                case CN_BIND:
                    if(snode->set.var)
                        VAR(set.var);
                    if(snode->set.value)
                        NODE(set.value);
                    break;
                case CN_LAMBDA:
                    FUNC(lambda.function);
                    NODES(lambda.closure);
                    dnode->lambda.function->node = dnode;
                    break;
                case CN_RETURN:
                    NODE(ret.value);
                    if(exits)
                        array_push(exits, dnode);
                    break;
                case CN_REF:
                    VAR(ref.var);
                    break;
                case CN_IF:
                    NODE(ifelse.cond);
                    break;
                case CN_PHI:
                    NODES(phi.args);
                    break;
                case CN_BUILTIN:
                    COPY(builtin.bi);
                    COPY(builtin.optype);
                    NODES(builtin.args);
                    break;
                case CN_COPY:
                    NODE(copy.value);
                    break;
                case CN_CONST:
                    COPY(constant);
                    break;
            }
        }
    }
}
static inline cnode_t *nil_actual(cnode_t *site, rtype_t *adecl)
{
    cnode_t *node = cnode_insert_before(site, CN_CONST);
    node->constant = c_intern(nil_init(adecl));
    node->decl = adecl;
    cnode_add_user(site, node);
    return node;
}
static cresult enforce_inline_args(cnode_t *site, cfunction_t *fn)
{
    cnode_array_t *args = &site->call.args;
    funsig_t *sig = fn->cl_type->sig;
    cresult res = SUCCESS;
    int i;

    array_foreach(args, i)
    {
        cnode_t **ptr = aptr(args, i);
        rtype_t *adecl = sig->args[i].type;

        if(!*ptr)
            *ptr = nil_actual(site, adecl);
        else
            res |= enforce_decl(site, adecl, ptr, false);
    }
    assert(cnode_compat(site, sig->ret_type) == YES);
    return res;
}
static void bind_closed_values(cblock_t *block, cfunction_t *caller,
                               cfunction_t *callee)
{
    cvar_array_t *vars = &callee->closure;
    cnode_array_t *vals = &callee->node->lambda.closure;
    int i;

    array_foreach(vars, i)
    {
        cvar_t *var = aref(vars, i);
        cnode_t *val = aref(vals, i), *node;

        if(!val)
            continue;
        if(var->local.binder != caller)
            continue;
        node = cnode_append(block, CN_BIND);
        node->decl = val->decl;
        node->set.var = var;
        node->set.value = val;
        cnode_add_user(node, val);
    }
}
static void bind_args(cnode_t *site, cblock_t *block, cfunction_t *caller,
                      cfunction_t *callee)
{
    cnode_array_t *args = &site->call.args;
    cnode_t *node;
    int nargs = 0;

    list_foreach_entry(&block->cnode_head, node, cnode_list)
    {
        if(node->type != CN_BIND)
            continue;
        if(!node->set.var)
        {
            cnode_reset(node);
            node->type = CN_CONST;
            node->constant = c_intern(r_box(r_type_int, &site->call.argbits));
            node->decl = r_type_int;
        }
        else if(!node->set.value)
        {
            cnode_t *val = aref(args, nargs);

            node->set.value = val;
            cnode_add_user(node, val);
            cnode_replace_in_users(node, val);
            nargs++;
        }
    }
}
static cresult link_exits(cblock_t *block, cfunction_t *callee,
                          cnode_array_t *exits, cnode_t **pval)
{
    rtype_t *ret_type = callee->cl_type->sig->ret_type;
    cnode_t **ptr;
    cresult res = SUCCESS;

    assert(alen(exits) > 0);
    array_foreach_ptr(exits, ptr)
    {
        cnode_t *node = *ptr;
        assert(node->type == CN_RETURN);
        res |= enforce_decl(node, ret_type, &node->ret.value, false);
        *ptr = node->ret.value;
        link_blocks(block, node->block);
        cnode_remove(node);
    }
    if(alen(exits) > 1)
    {
        cnode_t *arg;

        *pval = cnode_prepend(block, CN_PHI);
        (*pval)->phi.args = *exits;
        array_foreach_entry(exits, arg)
            cnode_add_user(*pval, arg);
    }
    else
    {
        *pval = aref(exits, 0);
        array_fini(exits);
    }
    return res;
}
static cresult expand_inline(cnode_t *site, cfunction_t *caller,
                             cfunction_t *callee)
{
    cresult res = SUCCESS;
    cnode_array_t exits = ARRAY_INIT;
    cblock_t *entry, *above, *below;
    cnode_t *value;

    array_init(&exits, 1);
    entry = copy_into(caller, callee, NULL, &exits);
    caller->ninlined += callee->nnodes;
    res |= enforce_inline_args(site, callee);
    below = site->block;
    above = split_block(caller, site);
    if(cfunc_has_closure(callee))
        bind_closed_values(above, caller, callee);
    bind_args(site, entry, caller, callee);
    link_blocks(entry, above);
    res |= link_exits(below, callee, &exits, &value);
    cnode_replace_in_users(site, value);
    cnode_remove(site);
    return res | CHANGED;
}
static float args_spec(cnode_array_t *args, funsig_t *sig)
{
    float factor = 0;
    int i, n = alen(args) > 0 ? alen(args) : 1;
    assert(alen(args) == sig->nargs);

    array_foreach(args, i)
    {
        cnode_t *arg = aref(args, i);
        rtype_t *atyp = decl_type(arg ? arg->decl : NULL);
        rtype_t *ftyp = sig->args[i].type;

        if(atyp != ftyp && r_subtypep(atyp, ftyp))
            factor += 1;
    }
    return factor / n;
}
static float args_const(cnode_array_t *args)
{
    float factor = 0;
    int i, n = alen(args) > 0 ? alen(args) : 1;

    array_foreach(args, i)
    {
        cnode_t *arg = aref(args, i);

        if(arg && arg->type == CN_CONST)
            factor += 1;
    }
    return factor / n;
}
#define WEIGHT_FUNC 8
#define WEIGHT_NODE 1
#define WEIGHT_SPEC 0.5
#define WEIGHT_CONST 0.25
static float score_function(cnode_t *site, cfunction_t *fn)
{
    float factor, score;

    score = fn->nfuncs * WEIGHT_FUNC + fn->nnodes * WEIGHT_NODE;
    factor = (args_spec(&site->call.args, fn->cl_type->sig) * WEIGHT_SPEC +
              args_const(&site->call.args) * WEIGHT_CONST);
    score -= score * factor;
    return score;
}
#define MAX_NODES 256
#define MIN_NODES 8
#define CUTOFF 32
static bool should_inline(cnode_t *site, cfunction_t *caller,
                          cfunction_t *callee)
{
    if(caller->ninlined + callee->nnodes > MAX_NODES)
        return false;
    if(cnode_only_used_by(callee->node, site))
        return true;
    if(callee->nfuncs == 0 && callee->nnodes < MIN_NODES)
        return true;
    return score_function(site, callee) < CUTOFF;
}
static bool can_inline(cnode_t *site, cfunction_t *caller, cfunction_t *callee)
{
    if(caller == callee)
        return false;
    if(!caller->parent)
       return false;
    if(!cfunc_has_closure(callee))
        return true;
    if(callee->parent == caller)
        return true;
    return false;
}
cresult inline_lambdas(cfunction_t *caller, cnode_array_t *nodes)
{
    cresult res = SUCCESS;
    cnode_t *node;

    if(!opt.opt_inline)
        return SUCCESS;
    array_foreach_entry(nodes, node)
    {
        assert(cell_const_lambda(cell_for(node->call.target)));
        cfunction_t *callee = cell_const(cell_for(node->call.target));
        funsig_t *sig = callee->cl_type->sig;

        if(can_inline(node, caller, callee)
           && call_normalise(node, sig) == CHANGED
           && should_inline(node, caller, callee))
            res |= expand_inline(node, caller, callee);
    }
    return res;
}
