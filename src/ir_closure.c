#include "global.h"
#include "ir.h"
cresult cfunc_populate_closure(cfunction_t *fn)
{
    cresult res = SUCCESS;
    cnode_t *def = fn->node;
    int i;

    array_resize(&def->lambda.closure, alen(&fn->closure));
    array_foreach(&fn->closure, i)
    {
        cvar_t *var = aref(&fn->closure, i);
        cnode_t **pnode = aptr(&def->lambda.closure, i), *node;

        if(!var->is_const || *pnode)
            continue;

        node = cnode_insert_before(def, CN_REF);
        node->ref.var = var;
        cnode_add_user(def, node);
        *pnode = node;
        res |= CHANGED;
    }
    return res;
}
static inline bool capturep(cvar_t *var, cfunction_t *fn)
{
    cvar_t *chk;

    if(var->local.binder == fn)
        return false;
    array_foreach_entry(&fn->closure, chk)
        if(chk == var)
            return false;
    return true;
}
static void add_closed_vars(cfunction_t *fn)
{
    cfunction_t *child;
    cvar_t *var;

    list_foreach_entry(&fn->cfunc_head, child, cfunc_list)
        array_foreach_entry(&child->closure, var)
            if(capturep(var, fn))
                array_push(&fn->closure, var);
}
static void add_free_vars(cfunction_t *fn)
{
    cblock_t *block;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        cnode_t *node;

        list_foreach_entry(&block->cnode_head, node, cnode_list)
        {
            cvar_t *var;

            if(node->type == CN_REF)
                var = node->ref.var;
            else if(node->type == CN_SET)
                var = node->set.var;
            else
                continue;
            if(cvar_is_global(var) || !capturep(var, fn))
                continue;
            var->local.is_closed = true;
            array_push(&fn->closure, var);
        }
    }
}
void cfunc_init_closure(cfunction_t *fn)
{
    add_closed_vars(fn);
    add_free_vars(fn);
    if(fn->node)
        cfunc_populate_closure(fn);
}
