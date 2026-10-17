#include "global.h"
#include "ir.h"
cfunction_t *cfunc_create(cfunction_t *parent)
{
    cfunction_t *fn = xcalloc(1, sizeof(*fn));

    fn->parent = parent;
    list_init(&fn->cvar_head);
    list_init(&fn->cfunc_head);
    list_init(&fn->cblock_head);
    array_init(&fn->closure, 0);
    if(parent)
        list_add(&parent->cfunc_head, &fn->cfunc_list);
    else
        list_init(&fn->cfunc_list);
    return fn;
}
void cfunc_free(cfunction_t *fn)
{
    cblock_t *block, *btmp;
    cvar_t *var, *vtmp;

    if(fn->parent)
        list_remove(&fn->cfunc_list);
    list_foreach_entry_safe(&fn->cblock_head, block, btmp, cblock_list)
        cblock_free(block);
    list_foreach_entry_safe(&fn->cvar_head, var, vtmp, cvar_list)
        cvar_free(var);
    array_fini(&fn->closure);
    xfree(fn);
}
rtype_t *cfunc_type(cfunction_t *fn, rtype_t *ret_type)
{
    int i = 0;
    cvar_t *var;

    list_foreach_entry(&fn->cvar_head, var, cvar_list)
        if(!cvar_is_global(var) && var->local.is_arg)
            i++;

    argdesc_t *args = alloca(sizeof(argdesc_t) * i);

    i = 0;
    list_foreach_entry(&fn->cvar_head, var, cvar_list)
    {
        if(cvar_is_global(var) || !var->local.is_arg)
            continue;
        args[i++] = argdesc_init(var->name, decl_type(var->decl),
                                 var->local.is_optional);
    }
    return rcall_type_create(decl_type(ret_type), i, args);
}
cresult cfunc_map_children(cfunction_t *fn, cresult (*func)(cfunction_t *))
{
    cfunction_t *child;
    cresult res = SUCCESS;

    list_foreach_entry(&fn->cfunc_head, child, cfunc_list)
        res |= func(child);
    return res;
}
void cfunc_mapc_children(cfunction_t *fn, void (*func)(cfunction_t *))
{
    cfunction_t *child;

    list_foreach_entry(&fn->cfunc_head, child, cfunc_list)
        func(child);
}
cblock_t *cblock_create()
{
    cblock_t *block = xcalloc(1, sizeof(*block));

    array_init(&block->pred, 0);
    array_init(&block->succ, 0);
    list_init(&block->cnode_head);
    return block;
}
void cblock_free(cblock_t *block)
{
    cnode_t *node, *tmp;

    list_remove(&block->cblock_list);
    list_foreach_entry_safe(&block->cnode_head, node, tmp, cnode_list)
        cnode_free(node);
    array_fini(&block->pred);
    array_fini(&block->succ);
    xfree(block);
}
int index_of_block(cblock_array_t *arr, cblock_t *block)
{
    int i;
    array_foreach(arr, i)
    {
        cblock_t *chk = aref(arr, i);
        if(chk == block)
            return i;
    }
    return -1;
}
void link_blocks(cblock_t *to, cblock_t *from)
{
    array_push(&from->succ, to);
    array_push(&to->pred, from);
}
void replace_link(cblock_array_t *arr, cblock_t *from, cblock_t *to)
{
    int i = index_of_block(arr, from);
    assert(i != -1);
    aset(arr, i, to);
}
void remove_link(cblock_array_t *arr, cblock_t *from)
{
    int i = index_of_block(arr, from);
    assert(i != -1);
    array_remove(arr, i);
}
cblock_t *split_block(cfunction_t *fn, cnode_t *site)
{
    cblock_t *pred, *block = site->block, *above = cblock_create();
    cnode_t *node, *tmp;

    assert(site->type != CN_PHI);
    array_foreach_entry(&block->pred, pred)
        replace_link(&pred->succ, block, above);
    above->pred = block->pred;
    array_init(&block->pred, 0);
    list_foreach_entry_safe(&block->cnode_head, node, tmp, cnode_list)
    {
        if(node == site)
            break;
        list_remove(&node->cnode_list);
        node->block = above;
        list_add_before(&above->cnode_head, &node->cnode_list);
    }
    list_add_before(&fn->cblock_head, &above->cblock_list);
    if(fn->entry == block)
        fn->entry = above;
    return above;
}
cnode_t *cnode_create(cblock_t *block, cnodetype type)
{
    cnode_t *node = xcalloc(1, sizeof(*node));

    *node = (cnode_t) {
        .block = block,
        .type = type
    };
    list_init(&node->cnode_list);
    array_init(&node->users, 0);
    return node;
}
cnode_t *cnode_append(cblock_t *block, cnodetype type)
{
    cnode_t *node = cnode_create(block, type);
    list_add_before(&block->cnode_head, &node->cnode_list);
    return node;
}
cnode_t *cnode_prepend(cblock_t *block, cnodetype type)
{
    cnode_t *node = cnode_create(block, type);
    list_add(&block->cnode_head, &node->cnode_list);
    return node;
}
cnode_t *cnode_insert_before(cnode_t *other, cnodetype type)
{
    cnode_t *node = cnode_create(other->block, type);
    list_add_before(&other->cnode_list, &node->cnode_list);
    return node;
}
cnode_t *cnode_insert_after(cnode_t *other, cnodetype type)
{
    cnode_t *node = cnode_create(other->block, type);
    list_add(&other->cnode_list, &node->cnode_list);
    return node;
}
void cnode_fini(cnode_t *node)
{
    switch(node->type)
    {
    case CN_CALL:
        array_fini(&node->call.names);
        /* fallthrough */
    case CN_CALL_FAST:
        array_fini(&node->call.args);
        break;
    case CN_PHI:
        array_fini(&node->phi.args);
        break;
    case CN_LAMBDA:
        cfunc_free(node->lambda.function);
        array_fini(&node->lambda.closure);
        break;
    case CN_BUILTIN:
        array_fini(&node->builtin.args);
        break;
    default:
        break;
    }
}
void cnode_free(cnode_t *node)
{
    cnode_fini(node);
    list_remove(&node->cnode_list);
    array_fini(&node->users);
    xfree(node);
}
void cnode_reset(cnode_t *node)
{
    node->decl = NULL;
    cnode_unuse_all(node);
    cnode_fini(node);
}
void cnode_remove(cnode_t *node)
{
    assert(alen(&node->users) == 0);
    cnode_unuse_all(node);
    cnode_free(node);
}
static inline bool call_is_pure(cnode_t *node, bool may_alias)
{
    if(node->call.target->type == CN_CONST)
    {
        robject_t *val = node->call.target->constant;

        if(rtype_is_callable(r_typeof(val))
           && rcall_is_builtin((rcallable_t *)val))
        {
            const cbuiltin_t *bi = ((rbuiltin_t *)val)->cbi;

            return bi && builtin_is_pure(bi, node, may_alias);
        }
    }
    return false;
}
bool cnode_is_pure(cnode_t *node, bool may_alias)
{
    switch(node->type)
    {
    case CN_LAMBDA:
    case CN_CONST:
    case CN_REF:
    case CN_PHI:
    case CN_COPY:
        return true;
    case CN_CALL:
    case CN_CALL_FAST:
        return call_is_pure(node, may_alias);
    case CN_BUILTIN:
        return builtin_is_pure(node->builtin.bi, node, may_alias);
    default:
        return false;
    }
}
int index_of_node(cnode_array_t *arr, cnode_t *node)
{
    int i;

    array_foreach(arr, i)
    {
        cnode_t *chk = aref(arr, i);
        if(chk == node)
            return i;
    }
    return -1;
}
#define NODE(f) func(&node->f, data)
#define NODES(f)                                \
    array_foreach_ptr(&node->f, ptr) {          \
        if(*ptr)                                \
            func(ptr, data);                    \
    }
void cnode_map_used(cnode_t *node, void (*func)(cnode_t **, void *),
                    void *data)
{
    cnode_t **ptr;

    switch(node->type)
    {
    case CN_IF: NODE(ifelse.cond); break;
    case CN_RETURN: NODE(ret.value); break;
    case CN_COPY: NODE(copy.value); break;
    case CN_CALL:
    case CN_CALL_FAST:
        NODE(call.target);
        NODES(call.args);
        break;
    case CN_BIND:
        if(!node->set.value)
            break;
        /* fallthrough */
    case CN_SET: NODE(set.value); break;
    case CN_PHI: NODES(phi.args); break;
    case CN_BUILTIN: NODES(builtin.args); break;
    case CN_LAMBDA: NODES(lambda.closure); break;
    default: break;
    }
}
void cnode_add_user(cnode_t *user, cnode_t *node)
{
    array_push(&node->users, user);
}
void cnode_remove_user(cnode_t *user, cnode_t *node)
{
    int i;

    array_foreach(&node->users, i)
    {
        if(aref(&node->users, i) == user)
        {
            array_remove(&node->users, i);
            return;
        }
    }
}
static void add_one(cnode_t **ptr, void *data)
    { cnode_add_user(data, *ptr); }
void cfunc_node_users(cfunction_t *fn)
{
    cblock_t *block;
    cnode_t *node;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
        list_foreach_entry(&block->cnode_head, node, cnode_list)
            cnode_map_used(node, add_one, node);
}
static void remove_one(cnode_t **ptr, void *data)
    { cnode_remove_user(data, *ptr); }
void cnode_unuse_all(cnode_t *node)
    { cnode_map_used(node, remove_one, node); }
typedef struct
{
    cnode_t *from, *to, *node;
} replace_ctx_t;
static void replace_one(cnode_t **ptr, void *data)
{
    replace_ctx_t *ctx = data;

    if(*ptr == ctx->from)
    {
        cnode_add_user(ctx->node, ctx->to);
        *ptr = ctx->to;
    }
}
void cnode_replace_in_users(cnode_t *from, cnode_t *to)
{
    replace_ctx_t ctx = { from, to };
    cnode_t *node;

    array_foreach_entry(&from->users, node)
    {
        ctx.node = node;
        cnode_map_used(node, replace_one, &ctx);
    }
    array_resize(&from->users, 0);
}
robject_t *nil_init(rtype_t *decl)
{
    robject_t *obj = NULL;

    if(decl && rtype_is_scalar(decl))
    {
        rvalue_union_t val = { 0 };
        obj = r_box(decl, &val);
    }
    return obj;
}
cnode_t *guard_decl(cnode_t *guard, cnode_t *node, rtype_t *decl,
                    cnode_t *val)
{
    guard->copy.value = val;
    guard->decl = decl_type(decl);
    guard->file = val->file;
    cnode_remove_user(node, val);
    cnode_add_user(node, guard);
    cnode_add_user(guard, val);
    return guard;
}
static cnode_t *guard_val(cnode_t *guard, cnode_t *node, rtype_t *decl)
{
    guard->copy.value = node;
    guard->decl = decl_type(decl);
    guard->file = node->file;
    cnode_replace_in_users(node, guard);
    cnode_add_user(guard, node);
    return guard;
}
cresult enforce_decl(cnode_t *node, rtype_t *decl, cnode_t **pval,
                     bool boxing)
{
    switch(cnode_compat(*pval, decl))
    {
    case YES:
        break;
    case NO:
        c_error("value of type `%s` found where type `%s` expected.",
                decl_name((*pval)->decl), decl_name(decl));
        return FAILED;
    case MAYBE:
        if(!boxing && decl_type(decl) == r_type_object)
            break;
        *pval = guard_decl(cnode_insert_before(node, CN_COPY),
                           node, decl, *pval);
        return CHANGED;
    }
    return SUCCESS;
}
cresult enforce_val(cnode_t *node, rtype_t *decl)
{
    switch(cnode_compat(node, decl))
    {
    case YES:
        break;
    case NO:
        c_error("value of type `%s` found where type `%s` expected.",
                decl_name(node->decl), decl_name(decl));
        return FAILED;
    case MAYBE:
        guard_val(cnode_insert_after(node, CN_COPY), node, node->decl);
        node->decl = decl;
        return CHANGED;
    }
    return SUCCESS;
}
cvar_t *cvar_create(rsymbol_t *name, cvartype type, rtype_t *decl)
{
    cvar_t *var = xcalloc(1, sizeof(*var));

    *var = (cvar_t) {
        .name = name,
        .decl = decl,
        .type = type,
    };
    list_init(&var->cvar_list);
    return var;
}
void cvar_free(cvar_t *var)
{
    list_remove(&var->cvar_list);
    xfree(var);
}
int index_of_var(cvar_array_t *arr, cvar_t *var)
{
    int i;

    array_foreach(arr, i)
    {
        cvar_t *chk = aref(arr, i);
        if(chk == var)
            return i;
    }
    return -1;
}
