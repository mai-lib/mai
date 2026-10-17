#include "global.h"
#include "ir.h"
#include "opt.h"
static cresult enforce_signature(cnode_t *node, rtype_t *type)
{
    funsig_t *sig = node->type == CN_CALL ? NULL : type->sig;
    cnode_array_t *args = &node->call.args;
    cresult res = SUCCESS;
    int i;

    assert(node->type == CN_CALL || rtype_is_callable(type));
    array_foreach(args, i)
    {
        cnode_t **ptr = aptr(args, i);
        rtype_t *decl = sig ? sig->args[i].type : r_type_object;

        if(*ptr)
            res |= enforce_decl(node, decl, ptr, true);
    }
    res |= enforce_val(node, sig ? sig->ret_type : r_type_object);
    return res;
}
static cresult enforce_fastcall(cnode_t *node, rtype_t *type)
{
    assert(type && type->sig);
    if((node->call.argbits & type->sig->reqbits) != type->sig->reqbits)
    {
        c_error("required argument not supplied.");
        return FAILED;
    }
    return enforce_signature(node, type);
}
static cresult enforce_call(cnode_t *node, rtype_t *type)
{
    cresult res = SUCCESS;

    if(type && type != r_type_object && type != r_type_callable)
    {
        if(!rtype_is_callable(type))
        {
            c_error("value of type `%s` is not callable.", decl_name(type));
            return FAILED;
        }
        res |= call_normalise(node, type->sig);
    }
    return res | enforce_signature(node, type);
}
static cresult enforce_phi(cnode_t *node, rtype_t *decl)
{
    int i;
    cresult res = SUCCESS;

    array_foreach(&node->phi.args, i)
    {
        cnode_t **pval = aptr(&node->phi.args, i);

        if(cnode_compat(*pval, decl) == MAYBE)
        {
            cblock_t *pred = aref(&node->block->pred, i);

            *pval = guard_decl(cnode_append(pred, CN_COPY),
                               node, decl, *pval);
            res |= CHANGED;
        }
    }
    return res;
}
static cresult cfunc_post_enforce(cfunction_t *fn)
{
    cblock_t *block;
    cnode_t *node, *tmp;
    cresult res = SUCCESS;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        list_foreach_entry_safe(&block->cnode_head, node, tmp, cnode_list)
        {
            switch(node->type)
            {
                case CN_IF:
                    res |= enforce_decl(node, r_type_boolean,
                                        &node->ifelse.cond, true);
                    break;
                case CN_RETURN:
                    res |= enforce_decl(node, fn->cl_type->sig->ret_type,
                                        &node->ret.value, true);
                    break;
                case CN_CALL:
                    res |= enforce_call(node, node->call.target->decl);
                    break;
                case CN_CALL_FAST:
                    res |= enforce_fastcall(node, node->call.target->decl);
                    break;
                case CN_SET:
                    res |= enforce_decl(node, node->set.var->decl, &node->set.value, true);
                    break;
                case CN_BUILTIN:
                {
                    const cbuiltin_t *bi = node->builtin.bi;

                    if(bi->ops->enforce_fn)
                        res |= bi->ops->enforce_fn(node);
                    break;
                }
                case CN_PHI:
                    res |= enforce_phi(node, node->decl);
                    break;
            default:
                break;
            }
        }
    }
    return res;
}
static cresult copy_of_const(cnode_t *node, robject_t *obj)
{
    bool valid;

    obj = constant_convert(obj, node->decl, &valid);
    if(valid)
    {
        node_become_constant(node, obj);
        return CHANGED;
    }
    return SUCCESS;
}
static cresult update_copy(cnode_t *node, cnode_t *val)
{
    switch(cnode_compat(val, node->decl))
    {
    case NO:
        c_error("value of type `%s` found where type `%s` expected.",
                decl_name(val->decl), decl_name(node->decl));
        return FAILED;
    case YES:
        cnode_replace_in_users(node, val);
        cnode_remove(node);
        return CHANGED;
    case MAYBE:
        break;
    }
    if(val->type == CN_CONST)
        return copy_of_const(node, val->constant);
    return SUCCESS;
}
static cresult cfunc_post_copy(cfunction_t *fn)
{
    cblock_t *block;
    cnode_t *node, *tmp;
    cresult res = SUCCESS;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        list_foreach_entry_safe(&block->cnode_head, node, tmp, cnode_list)
        {
            if(node->type == CN_COPY)
                res |= update_copy(node, node->copy.value);
        }
    }
    return res;
}
static cresult cfunc_postpass(cfunction_t *fn)
{
    cresult r, res = cfunc_post_enforce(fn);

    do res |= (r = cfunc_post_copy(fn));
    while(r == CHANGED);
    if(changed(res))
    {
        cfunc_cleanup(fn);
        cfunc_rdfo(fn);
    }
    return res;
}
cresult ir_postpass(cfunction_t *fn)
{
    return cfunc_postpass(fn) | cfunc_map_children(fn, ir_postpass);
}
