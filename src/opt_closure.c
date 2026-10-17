#include "global.h"
#include "ir.h"
#include "opt.h"
#include "vm/vm_ops.h"
#include "gen_code.h"
static void gen_cell_make(cnode_t *node)
{
    asm_op(OP_cellmake);
    asm_stack(loc_for(node));
    asm_offset(const_index(node->builtin.optype));
}
static const cbuiltin_t cell_make = {
    &(builtin_ops_t) { .generate_fn = gen_cell_make },
    NULL, "cell_make"
};
static void gen_cell_get(cnode_t *node)
{
    asm_op_width(OP_cellget, node->builtin.optype->elt);
    asm_stack(loc_for(node));
    asm_args(&node->builtin.args);
}
static const cbuiltin_t cell_get = {
    &(builtin_ops_t) { .generate_fn = gen_cell_get },
    NULL, "cell_get"
};
static cresult enforce_cell_set(cnode_t *node)
{
    cnode_array_t *args = &node->builtin.args;
    assert(cnode_compat(aref(args, 0), node->builtin.optype));
    return enforce_decl(node, node->builtin.optype->elt, aptr(args, 1), true);
}
static void gen_cell_set(cnode_t *node)
{
    asm_op_width(OP_cellset, node->builtin.optype->elt);
    asm_args(&node->builtin.args);
}
static const cbuiltin_t cell_set = {
    &(builtin_ops_t) { .enforce_fn = enforce_cell_set,
                       .generate_fn = gen_cell_set,
                       .is_void = true },
    NULL, "cell_set"
};
static inline rtype_t *cell_decl_type(cvar_t *var)
    { return rcell_type_create(decl_type(var->decl)); }
static inline void init_ref(cnode_t *node, cvar_t *var)
{
    node->ref.var = var;
    node->decl = cell_decl_type(var);
}
static inline void init_set(cnode_t *node, cvar_t *var, cnode_t *val)
{
    node->set.var = var;
    if(val)
    {
        node->set.value = val;
        cnode_add_user(node, val);
    }
}
static inline
void init_bind(cnode_t *node, cvar_t *var, cnode_t *val, rtype_t *decl)
{
    node->type = CN_BIND;
    init_set(node, var, val);
    node->decl = decl;
}
static inline
void init_cellaccess(cnode_t *node, cvar_t *var, const cbuiltin_t *bi,
                     cnode_t *arg1, cnode_t *arg2)
{
    cnode_array_t args = ARRAY_INIT;

    if(arg1)
    {
        array_push(&args, arg1);
        cnode_add_user(node, arg1);
    }
    if(arg2)
    {
        array_push(&args, arg2);
        cnode_add_user(node, arg2);
    }
    node->type = CN_BUILTIN;
    node->builtin.bi = bi;
    node->builtin.args = args;
    node->builtin.optype = cell_decl_type(var);
}
static inline
void init_cellset(cnode_t *node, cvar_t *var, cnode_t *cell, cnode_t *val)
{
    init_cellaccess(node, var, &cell_set, cell, val);
}
static inline void init_cellget(cnode_t *node, cvar_t *var, cnode_t *cell)
{
    init_cellaccess(node, var, &cell_get, cell, NULL);
    node->decl = node->builtin.optype->elt;
}
static inline void init_cellmake(cnode_t *node, cvar_t *var)
{
    init_cellaccess(node, var, &cell_make, NULL, NULL);
    node->decl = node->builtin.optype;
}
static void cell_intro_get(cnode_t *node, cvar_t *var)
{
    cnode_t *ref = cnode_insert_before(node, CN_REF);

    init_ref(ref, var);
    cnode_reset(node);
    init_cellget(node, var, ref);
}
static void cell_intro_set(cnode_t *node, cvar_t *var, cnode_t *val)
{
    cnode_t *ref = cnode_insert_before(node, CN_REF);

    init_ref(ref, var);
    cnode_reset(node);
    init_cellset(node, var, ref, val);
}
static void cell_intro_arg(cnode_t *node, cvar_t *var)
{
    cnode_t *bind = cnode_insert_before(node, CN_BIND);
    cnode_t *cset = cnode_insert_after(node, CN_BUILTIN);
    cnode_t *set = cnode_insert_after(cset, CN_SET);

    assert(var->decl == node->decl);
    init_bind(bind, var, NULL, var->decl);
    init_cellset(cset, var, node, bind);
    init_set(set, var, node);
    cnode_reset(node);
    init_cellmake(node, var);
}
static void cell_intro_local(cnode_t *node, cvar_t *var, cnode_t *val)
{
    cnode_t *cmake = cnode_insert_before(node, CN_BUILTIN);
    cnode_t *cset = cnode_insert_before(node, CN_BUILTIN);

    init_cellmake(cmake, var);
    init_cellset(cset, var, cmake, val);
    cnode_reset(node);
    init_bind(node, var, cmake, cmake->decl);
}
static cresult cfunc_cell_rewrite(cfunction_t *fn)
{
    cresult res = SUCCESS;
    cblock_t *block;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        cnode_t *node, *tmp;

        list_foreach_entry_safe(&block->cnode_head, node, tmp, cnode_list)
        {
            switch(node->type)
            {
                case CN_REF:
                {
                    cvar_t *var = node->ref.var;
                    if(!cvar_is_celled(var))
                        continue;
                    cell_intro_get(node, var);
                    res |= CHANGED;
                    break;
                }
                case CN_SET:
                case CN_BIND:
                {
                    cvar_t *var = node->set.var;
                    if(!var || !cvar_is_celled(var))
                        continue;
                    if(!node->set.value)
                        cell_intro_arg(node, var);
                    else if(node->type == CN_BIND)
                        cell_intro_local(node, var, node->set.value);
                    else
                        cell_intro_set(node, var, node->set.value);
                    res |= CHANGED;
                    break;
                }
            default:
                break;
            }
        }
    }
    return res;
}
static cresult ir_cell_rewrite(cfunction_t *fn)
{
    cresult res = cfunc_cell_rewrite(fn);
    if(changed(res))
        cfunc_rdfo(fn);
    return res | cfunc_map_children(fn, ir_cell_rewrite);
}
cresult cfunc_populate_closure(cfunction_t *fn);
static cresult ir_cell_finalize(cfunction_t *fn)
{
    cvar_t *var;
    cresult res;

    list_foreach_entry(&fn->cvar_head, var, cvar_list)
    {
        if(cvar_is_celled(var))
        {
            var->decl = cell_decl_type(var);
            var->is_const = true;
        }
    }
    res = cfunc_map_children(fn, cfunc_populate_closure);
    if(changed(res))
        cfunc_ssa_convert(fn);
    return res | cfunc_map_children(fn, ir_cell_finalize);
}
cresult ir_cell_intro(cfunction_t *fn)
{
    return ir_cell_rewrite(fn) | ir_cell_finalize(fn);
}
