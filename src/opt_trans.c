#include "global.h"
#include "ir.h"
#include "opt.h"
bool scalar_convert(void *dest, robject_t *obj, rtype_t *to, rtype_t *from);
bool builtin_is_pure(const cbuiltin_t *bi, cnode_t *node, bool may_alias)
    { return bi->ops && bi->ops->is_pure
             && bi->ops->is_pure(node, may_alias); }
bool builtin_is_void(const cbuiltin_t *bi, cnode_t *node)
    { return bi->ops && bi->ops->is_void; }
static cresult trans_builtin(opt_phase phase, cnode_t *node, cell_t *cell,
                             const cbuiltin_t *bi, cnode_array_t *args)
{
    if(bi && bi->ops && bi->ops->trans_fn)
        return bi->ops->trans_fn(phase, node, cell, bi, args);
    return SUCCESS;
}
static cresult maybe_trans_builtin(opt_phase phase, cnode_t *node,
                                  cell_t *cell, cell_t *target)
{
    if(opt.opt_builtin && cell_const_obj(target)
       && node->type == CN_CALL)
    {
        robject_t *obj = cell_const(target);
        if(rtype_is_callable(r_typeof(obj)))
        {
            rcallable_t *cl = (rcallable_t *)obj;
            if(rcall_is_builtin(cl))
            {
                rbuiltin_t *rbi = (rbuiltin_t *)obj;
                return trans_builtin(phase, node, cell, rbi->cbi,
                                     &node->call.args);
            }
        }
    }
    return SUCCESS;
}
robject_t *constant_convert(robject_t *sval, rtype_t *dtype, bool *pvalid)
{
    rtype_t *stype = r_typeof(sval);

    *pvalid = false;
    if(stype == dtype)
    {
        *pvalid = true;
        return sval;
    }
    if(rtype_is_scalar(dtype))
    {
        if(rtype_is_scalar(stype))
        {
            rvalue_union_t val;

            *pvalid = true;
            scalar_convert(&val, sval, dtype, stype);
            return c_intern(r_box(dtype, &val));
        }
    }
    else if(rtype_is_scalar(stype))
    {
        if(dtype == r_type_object)
        {
            *pvalid = true;
            return sval;
        }
    }
    else if(r_subtypep(stype, dtype))
    {
        *pvalid = true;
        return sval;
    }
    return NULL;
}
static inline void set_const_global(cell_t *cell, rglobal_t *global)
{
    rtype_t *type = global->decl;
    robject_t *obj = rtype_is_scalar(type)
        ? c_intern(r_box(type, &global->val))
        : global->val.object;

    cell_set_const(cell, obj);
}
static inline void set_const_value(cell_t *cell, cnode_t *node)
{
    switch(node->type)
    {
    case CN_CONST:
        cell_set_const_at(cell, node->constant, node->decl);
        break;
    case CN_LAMBDA:
        cell_set_lambda(cell, node->lambda.function);
        break;
    default:
        cell_set_type(cell, decl_type(node->decl));
        break;
    }
}
static inline void set_const_closed(cell_t *cell, cvar_t *var, cfunction_t *fn)
{
    int i = index_of_var(&fn->closure, var);
    set_const_value(cell, aref(&fn->node->lambda.closure, i));
}
static void transfer_ref(cell_t *cell, cvar_t *var, cfunction_t *fn)
{
    if(!var->is_const)
    {
        if(var->decl)
            cell_set_type(cell, var->decl);
        return;
    }
    switch(var->type)
    {
    case GLOBAL_INT:
        set_const_value(cell, var->intl.set->set.value);
        return;
    case GLOBAL_EXT:
        set_const_global(cell, var->extl.global);
        return;
    case LEXICAL:
        set_const_closed(cell, var, fn);
        break;
    }
}
static inline void transfer_copy(cell_t *cell, cnode_t *src, rtype_t *type)
{
    cell_t *scell = cell_for(src);
    rtype_t *styp = cell_type(scell);

    if(cell_const_obj(scell))
    {
        robject_t *val = cell_const(scell);
        bool valid;

        val = constant_convert(val, type, &valid);
        if(valid)
        {
            cell_set_const(cell, val);
            return;
        }
    }
    else if(cell_const_lambda(scell))
    {
        cfunction_t *fn = cell_const(scell);

        if(r_subtypep(fn->cl_type, type))
        {
            cell_set_lambda(cell, fn);
            return;
        }
    }
    if(styp && r_subtypep(styp, type))
        cell_set_type(cell, styp);
    else
        cell_set_type(cell, type);
}
static void transfer_call(cnode_t *node, cell_t *cell)
{
    cell_t *target = cell_for(node->call.target);
    rtype_t *type = cell_type(target);

    if(!type || !rtype_is_callable(type))
        return;
    cell_set_type(cell, type->sig->ret_type);
    maybe_trans_builtin(TRANSFER, node, cell, target);
}
void opt_transfer(cfunction_t *fn, cnode_t *node, cell_t *cell)
{
    switch(node->type)
    {
        case CN_LAMBDA:
            cell_set_lambda(cell, node->lambda.function);
            break;
        case CN_CONST:
            cell_set_const_at(cell, node->constant, node->decl);
            break;
        case CN_BIND:
            if(!node->set.value && node->decl)
                cell_set_type(cell, node->decl);
            break;
        case CN_CALL_FAST:
        case CN_CALL:
            transfer_call(node, cell);
            break;
        case CN_REF:
            transfer_ref(cell, node->ref.var, fn);
            break;
        case CN_COPY:
            transfer_copy(cell, node->copy.value, node->decl);
            break;
        case CN_BUILTIN:
            trans_builtin(TRANSFER, node, cell, node->builtin.bi,
                          &node->builtin.args);
            break;
    default:
        break; /* NOTREACHED */
    }
}
void call_become_builtin(cnode_t *node, const cbuiltin_t *bi,
                         rtype_t *optype, rtype_t *type)
{
    cnode_array_t args = node->call.args;

    node->call.args = (cnode_array_t) ARRAY_INIT;
    cnode_reset(node);
    node->type = CN_BUILTIN;
    node->decl = type;
    node->builtin.bi = bi;
    node->builtin.args = args;
    node->builtin.optype = optype;
}

void node_become_constant(cnode_t *node, robject_t *val)
{
    rtype_t *type = node->decl;

    cnode_reset(node);
    node->type = CN_CONST;
    node->decl = type;
    node->constant = val;
}
static cresult transform_call(cnode_t *node, cell_t *cell)
{
    cell_t *target = cell_for(node->call.target);

    if(opt.opt_inline && cell_const_lambda(target))
    {
        array_push(&inlines, node);
        return SUCCESS;
    }
    return maybe_trans_builtin(TRANSFORM, node, cell, target);
}
static cresult transform_phi(cnode_t *node)
{
    cblock_t *block = node->block;
    cnode_t *buf[alen(&block->pred)];
    bool purge = true;
    int i, j = 0;

    if(!opt.opt_dce)
        return SUCCESS;
    array_foreach(&block->pred, i)
    {
        cblock_t *pred = aref(&block->pred, i);
        cnode_t *arg = aref(&node->phi.args, i);

        if(flag_for(pred))
        {
            if(j > 0 && arg != buf[j-1])
                purge = false;
            buf[j++] = arg;
        }
        else
            cnode_remove_user(node, arg);
    }
    if(purge)
    {
        cnode_replace_in_users(node, buf[0]);
        cnode_remove(node);
        return CHANGED;
    }
    else if(j == alen(&block->pred))
        return SUCCESS;

    array_clear(&node->phi.args);
    for(i=0; i<j; i++)
        array_push(&node->phi.args, buf[i]);
    return CHANGED;
}
static cresult transform_if(cnode_t *node)
{
    cblock_t *block = node->block;
    cblock_t *succ;

    if(!opt.opt_dce)
        return SUCCESS;
    array_foreach_entry(&block->succ, succ)
    {
        if(!flag_for(succ))
        {
            cnode_remove(node);
            return CHANGED;
        }
    }
    return SUCCESS;
}
static cresult transform_set(cnode_t *node, cell_t *cell)
{
    cvar_t *var = node->set.var;

    if(var->type == GLOBAL_INT && var->is_const)
    {
        cell_t *value = cell_for(node->set.value);
        rtype_t *type = cell_type(value);

        if(type && var->decl != type)
        {
            var->decl = type;
            return CHANGED;
        }
    }
    return SUCCESS;
}
static inline void shuffle_phi(cnode_t *node)
{
    list_t *link;

    for(link = node->cnode_list.next;
        link != &node->block->cnode_head;
        link = link->next)
    {
        cnode_t *next = list_entry(link, node, cnode_list);

        if(next->type != CN_PHI)
            break;
    }
    list_remove(&node->cnode_list);
    list_add_before(link, &node->cnode_list);
}
static bool fold_constant(cnode_t *node, cell_t *cell)
{
    if(opt.opt_constfold && cell_const_obj(cell))
    {
        robject_t *val = cell_const(cell);

        if(node->type == CN_PHI)
            shuffle_phi(node);
        node_become_constant(node, val);
        return true;
    }
    return false;
}
static cresult update_decl(cnode_t *node, cell_t *cell)
{
    if(cnode_yields_value(node))
    {
        rtype_t *type = cell_type(cell);

        if(type && node->decl != type)
        {
            node->decl = type;
            return CHANGED;
        }
    }
    return SUCCESS;
}
cresult opt_transform(cfunction_t *fn, cnode_t *node, cell_t *cell)
{
    cresult res = update_decl(node, cell);

    switch(node->type)
    {
        case CN_REF:
        case CN_COPY:
        case CN_BUILTIN:
            if(fold_constant(node, cell))
                res |= CHANGED;
            break;
        case CN_CALL:
        case CN_CALL_FAST:
            if(fold_constant(node, cell))
                res |= CHANGED;
            else
                res |= transform_call(node, cell);
            break;
        case CN_PHI:
            if(fold_constant(node, cell))
                res |= CHANGED;
            else
                res |= transform_phi(node);
            break;
        case CN_SET:
            res |= transform_set(node, cell);
            break;
        case CN_IF:
            res |= transform_if(node);
            break;
    default:
        break;
    }
    return res;
}
