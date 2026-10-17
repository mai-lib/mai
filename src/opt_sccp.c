#include "global.h"
#include "worklist.h"
#include "ir.h"
#include "opt.h"
cell_t *cells;
bool *flags;
static WORKLIST(cnode_t *) val_work;
static WORKLIST(cblock_t *) ctrl_work;
cnode_array_t inlines;
static inline void opt_init(cfunction_t *fn)
{
    cells = xcalloc(fn->nnodes, sizeof(*cells));
    flags = xcalloc(fn->nblocks, sizeof(*flags));
    array_init(&inlines, 0);
}
static inline void opt_fini()
{
    array_fini(&inlines);
    xfree(cells);
    xfree(flags);
}
static inline cell_t cell_meet(cell_t *this, cell_t *other)
{
    if(this->state > other->state)
    {
        cell_t *tmp = this;
        this = other;
        other = tmp;
    }
    if(this->state == TOP || other->state == BOTTOM)
        return *other;
    if(other->state < TYPE
       && this->state == other->state
       && this->value == other->value)
    {
        if(this->type == other->type)
            return *this;
        return (cell_t) {
            .state = this->state,
            .value = this->value,
            .type = r_common_type(this->type, other->type)
        };
    }
    return (cell_t) {
        .state = TYPE,
        .type = r_common_type(this->type, other->type)
    };
}
static inline bool cell_changed(cell_t *this, cell_t *other)
{
    if(this->state != other->state)
        return true;
    bool chg = false;
    switch(this->state)
    {
    case CONST_OBJ:
    case CONST_LAMBDA:
        chg = (this->value != other->value);
        /* fallthrough */
    case TYPE:
        return chg | (this->type != other->type);
    default:
        return false;
    }
}
static inline void add_val_work(cnode_t *node)
    { worklist_push(&val_work, node, node->id); }
static inline void add_ctrl_work(cblock_t *dest)
{
    cnode_t *node;

    list_foreach_entry(&dest->cnode_head, node, cnode_list)
        if(node->type == CN_PHI)
            add_val_work(node);
        else
            break;
    worklist_push(&ctrl_work, dest, dest->id);
}
static void add_cond_work(cnode_t *node, cnode_t *user)
{
    cell_t *cell = cell_for(node);
    cblock_t *block = user->block;
    cblock_t *tb = aref(&block->succ, 0),
             *fb = aref(&block->succ, 1);

    if(cell_const_obj(cell)
       && r_typeof(cell_const(cell)) == r_type_boolean)
    {
        rboolean_t val = UNBOX(rboolean_t, cell_const(cell));
        if(val == true)
        {
            add_ctrl_work(tb);
            return;
        }
        else if(val == false)
        {
            add_ctrl_work(fb);
            return;
        }
    }
    add_ctrl_work(tb);
    add_ctrl_work(fb);
}
static inline void add_update_work(cnode_t *node)
{
    cnode_t *user;

    array_foreach_entry(&node->users, user)
    {
        if(user->type == CN_IF)
            add_cond_work(node, user);
        else if(cnode_yields_value(user))
            add_val_work(user);
    }
}
static inline bool set_flag_for(cblock_t *block)
{
    bool visited = flags[block->id];
    flags[block->id] = true;
    return visited;
}
static void visit_phi(cnode_t *node, cell_t *cell, cell_t *new)
{
    int i;

    cell_init(new, TOP);
    array_foreach(&node->phi.args, i)
    {
        if(flag_for(aref(&node->block->pred, i)))
        {
            cnode_t *arg = aref(&node->phi.args, i);

            *new = cell_meet(new, cell_for(arg));
        }
    }
}
static void visit_node(cfunction_t *fn, cnode_t *node, cell_t *cell, cell_t *new)
{
    cell_init(new, BOTTOM);
    opt_transfer(fn, node, new);
}
static void cfunc_sccp(cfunction_t *fn)
{
    bool run;

    worklist_init(&ctrl_work, fn->nblocks);
    worklist_init(&val_work, fn->nnodes);
    add_ctrl_work(fn->entry);
    do
    {
        run = false;
        for(cblock_t *block; worklist_take(&ctrl_work, &block, block->id); run = true)
        {
            cnode_t *node;

            if(set_flag_for(block))
                continue;
            list_foreach_entry(&block->cnode_head, node, cnode_list)
                if(node->type != CN_PHI && cnode_yields_value(node))
                    add_val_work(node);
            if(alen(&block->succ) == 1)
                add_ctrl_work(aref(&block->succ, 0));
        }
        for(cnode_t *node; worklist_take(&val_work, &node, node->id); run = true)
        {
            cell_t new, *cell = cell_for(node);

            if(node->type == CN_PHI)
                visit_phi(node, cell, &new);
            else if(flag_for(node->block))
                visit_node(fn, node, cell, &new);
            else
                continue;
            if(cell_changed(&new, cell))
            {
                *cell = new;
                add_update_work(node);
            }
        }
    } while(run);
    worklist_fini(&ctrl_work);
    worklist_fini(&val_work);
}
static void prune_block(cblock_t *block)
{
    cblock_t *other;
    cnode_t *node;

    array_foreach_entry(&block->pred, other)
        if(flag_for(other))
            remove_link(&other->succ, block);
    array_foreach_entry(&block->succ, other)
        if(flag_for(other))
            remove_link(&other->pred, block);
    list_foreach_entry(&block->cnode_head, node, cnode_list)
        cnode_unuse_all(node);
}
static cresult prune_dead_blocks(cblock_array_t *arr)
{
    cblock_t *block;

    if(!opt.opt_dce || alen(arr) == 0)
        return SUCCESS;
    array_foreach_entry(arr, block)
        prune_block(block);
    array_foreach_entry(arr, block)
        cblock_free(block);
    return CHANGED;
}
static cresult cfunc_optimise(cfunction_t *fn)
{
    cblock_array_t pruned;
    cresult res = SUCCESS;
    cblock_t *block;

    array_init(&pruned, 0);
    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        cnode_t *node, *tmp;

        if(!flag_for(block))
        {
            array_push(&pruned, block);
            continue;
        }
        list_foreach_entry_safe(&block->cnode_head, node, tmp, cnode_list)
            res |= opt_transform(fn, node, cell_for(node));
    }
    res |= prune_dead_blocks(&pruned);
    array_fini(&pruned);
    return res;
}
static void cfunc_closure_pre(cfunction_t *fn)
{
    cvar_array_t *vars = &fn->closure;
    cnode_array_t *vals = &fn->node->lambda.closure;
    int i;

    array_foreach(vars, i)
    {
        cvar_t *var = aref(vars, i);
        cnode_t *val = aref(vals, i);

        if(!val || !var->is_const)
            continue;
        var->decl = val->decl;
    }
}
static void cfunc_closure_post(cfunction_t *fn)
{
    cfunction_t *child;
    cvar_t *var;

    list_foreach_entry(&fn->cvar_head, var, cvar_list)
    {
        if(cvar_is_global(var))
            continue;
        var->local.is_closed = false;
    }
    list_foreach_entry(&fn->cfunc_head, child, cfunc_list)
    {
        array_foreach_entry(&child->closure, var)
            var->local.is_closed = true;
    }
}
cresult ir_optimise(cfunction_t *fn)
{
    cresult res = SUCCESS, ores, ires;

    if(fn->parent)
        cfunc_closure_pre(fn);
    do
    {
        if(fn->nnodes == 0 || fn->nblocks == 0)
            break;

        opt_init(fn);
        cfunc_sccp(fn);
        ores = cfunc_optimise(fn);
        res |= ores |= ires = inline_lambdas(fn, &inlines);
        opt_fini();
        if(ores == CHANGED)
        {
            cfunc_cleanup(fn);
            cfunc_rdfo(fn);
        }
        if(ires == CHANGED)
        {
            cfunc_closure_post(fn);
            cfunc_ssa_convert(fn);
        }
    } while(ores == CHANGED);
    return res | cfunc_map_children(fn, ir_optimise);
}
