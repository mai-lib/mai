#include "global.h"
#include "ir.h"
static void split_edge(cfunction_t *fn, cblock_t *block, cblock_t *succ)
{
    cblock_t *empty = cblock_create();

    list_add(&fn->cblock_head, &empty->cblock_list);
    array_push(&empty->succ, succ);
    array_push(&empty->pred, block);
    replace_link(&block->succ, succ, empty);
    replace_link(&succ->pred, block, empty);
}
cresult cfunc_crit_edges(cfunction_t *fn)
{
    cblock_t *block, *succ;
    cresult res = SUCCESS;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        if(alen(&block->succ) <= 1)
            continue;
        array_foreach_entry(&block->succ, succ)
        {
            if(alen(&succ->pred) <= 1)
                continue;
            split_edge(fn, block, succ);
            res |= CHANGED;
        }
    }
    return res;
}
#define make_room(arr, num, idx) do {                           \
        int len = alen(arr);                                    \
        array_resize(arr, len + num - 1);                       \
        memmove((arr)->ptr + (idx + num),                       \
                (arr)->ptr + (idx + 1),                         \
                (len - idx - 1) * sizeof(*(arr)->ptr));         \
    } while(0);
static inline void splice_preds(cblock_t *succ, cblock_t *block, int idx)
{
    cblock_array_t *arr = &succ->pred, *ins = &block->pred;
    int num = alen(ins);

    make_room(arr, num, idx);
    memcpy(arr->ptr + idx, ins->ptr, num * sizeof(cblock_t *));
}
static inline void splice_dup_args(cnode_t *node, int num, int idx)
{
    cnode_array_t *arr = &node->phi.args;
    cnode_t *ins = aref(arr, idx);

    make_room(arr, num, idx);
    for(int i = 1; i < num; i++)
    {
        cnode_add_user(node, ins);
        aset(arr, idx + i, ins);
    }
}
static void empty_splice(cblock_t *block)
{
    cblock_t *pred, *succ = aref(&block->succ, 0);
    int idx = index_of_block(&succ->pred, block);
    cnode_t *node;

    splice_preds(succ, block, idx);
    array_foreach_entry(&block->pred, pred)
        replace_link(&pred->succ, block, succ);
    list_foreach_entry(&succ->cnode_head, node, cnode_list)
    {
        if(node->type != CN_PHI)
            break;
        splice_dup_args(node, alen(&block->pred), idx);
    }
    cblock_free(block);
}
static void empty_source(cblock_t *block)
{
    cblock_t *succ = aref(&block->succ, 0);

    remove_link(&succ->pred, block);
    cblock_free(block);
}
static void empty_snip(cblock_t *block)
{
    cblock_t *succ = aref(&block->succ, 0),
             *pred = aref(&block->pred, 0);

    replace_link(&succ->pred, block, pred);
    replace_link(&pred->succ, block, succ);
    cblock_free(block);
}
static void coalesce_forward(cblock_t *block)
{
    cblock_t *pred, *succ = aref(&block->succ, 0);
    cnode_t *node;

    array_copy(&succ->pred, &block->pred);
    array_foreach_entry(&succ->pred, pred)
        replace_link(&pred->succ, block, succ);
    list_foreach_entry(&block->cnode_head, node, cnode_list)
        node->block = succ;
    list_splice_after(&succ->cnode_head, &block->cnode_head);
    cblock_free(block);
}
static void update_entry(cfunction_t *fn, cblock_t *block)
{
    if(block == fn->entry)
    {
        assert(alen(&block->succ) == 1);
        fn->entry = aref(&block->succ, 0);
        assert(fn->entry != block);
    }
}
typedef void (*blkclean_fn)(cblock_t *);
static inline blkclean_fn select_block_cleanup(cfunction_t *fn,
                                               cblock_t *block)
{
    int npred = alen(&block->pred);
    bool empty = list_isempty(&block->cnode_head);

    if(alen(&block->succ) != 1)
        return NULL;

    cblock_t *succ = aref(&block->succ, 0);
    int s_npred = alen(&succ->pred);
    assert(s_npred > 0);

    if(succ == block)
        return NULL;
    if(empty)
    {
        if(npred == 0)
            return empty_source;
        if(npred == 1)
        {
            cblock_t *pred = aref(&block->pred, 0);

            if(alen(&pred->succ) == 1 || s_npred == 1)
                return empty_snip;
        }
        else
        {
            if(alen(&succ->succ) < 2)
                return empty_splice;
        }
    }
    else if(s_npred == 1 && succ != fn->entry)
        return coalesce_forward;
    return NULL;
}
static cresult cfunc_clean_blocks(cfunction_t *fn)
{
    cresult res = SUCCESS;
    cblock_t *block, *btmp;

    list_foreach_entry_safe(&fn->cblock_head, block, btmp, cblock_list)
    {
        blkclean_fn clean = select_block_cleanup(fn, block);
        if(clean)
        {
            update_entry(fn, block);
            clean(block);
            res |= CHANGED;
        }
    }
    return res;
}
static unsigned tick = 1;
static void mark_used(cnode_t **ptr, void *data)
{
    cnode_t *node = *ptr;
    assert(node);
    assert(node->type != CN_SET);
    assert(node->type != CN_IF);
    assert(node->type != CN_RETURN);
    node->mark = tick;
}
static void mark_var(cnode_t *node)
{
    switch(node->type)
    {
    case CN_BIND:
    case CN_SET:
        if(node->set.var)
             node->set.var->mark = tick;
        break;
    case CN_REF:
        node->ref.var->mark = tick;
        break;
    default:
        break;
    }
}
static void cfunc_mark_used_nodes(cfunction_t *fn)
{
    cblock_t *block;
    cnode_t *node;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        list_foreach_entry(&block->cnode_head, node, cnode_list)
        {
            cnode_map_used(node, mark_used, NULL);
            mark_var(node);
        }
    }
}
static cresult cfunc_clean_unmarked_nodes(cfunction_t *fn)
{
    cresult res = SUCCESS;
    cblock_t *block;
    cnode_t *node, *ntmp;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        list_foreach_entry_safe(&block->cnode_head, node, ntmp, cnode_list)
        {
            if(node->mark != tick && cnode_is_pure(node, false))
            {
                cnode_remove(node);
                res |= CHANGED;
            }
        }
    }
    return res;
}
static cresult cfunc_clean_nodes(cfunction_t *fn)
{
    tick++;
    cfunc_mark_used_nodes(fn);
    return cfunc_clean_unmarked_nodes(fn);
}
static void cfunc_clean_vars(cfunction_t *fn)
{
    cvar_t *var, *tmp;

    list_foreach_entry_safe(&fn->cvar_head, var, tmp, cvar_list)
    {
        if(var->mark != tick && var->type == LEXICAL && !var->local.is_closed)
        {
            assert(!var->local.is_arg);
            cvar_free(var);
        }
    }
}
static inline cresult fix_cleanup(cfunction_t *fn,
                                  cresult (*clean)(cfunction_t *))
{
    cresult r, res = SUCCESS;
    do
    {
        r = clean(fn);
        res |= r;
    } while(changed(r));
    return res;
}
void cfunc_cleanup(cfunction_t *fn)
{
    fix_cleanup(fn, cfunc_clean_blocks);
    if(fix_cleanup(fn, cfunc_clean_nodes))
        fix_cleanup(fn, cfunc_clean_blocks);
    cfunc_clean_vars(fn);
}
static int cblock_number(cblock_t *block, int nnodes)
{
    cnode_t *node;

    if(!list_isempty(&block->cnode_head))
    {
        block->start = nnodes;
        list_foreach_entry(&block->cnode_head, node, cnode_list)
            node->id = nnodes++;
        block->end = nnodes - 1;
    }
    else
        block->start = block->end = INT_MIN;
    return nnodes;
}
static void cfunc_number(cfunction_t *fn)
{
    cblock_t *block;
    cfunction_t *child;
    int nblocks = 0, nnodes = 0, nfuncs = 0;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        nnodes = cblock_number(block, nnodes);
        block->id = nblocks++;
    }
    fn->nblocks = nblocks;
    fn->nnodes = nnodes;
    assert(nblocks > 0);
    list_foreach_entry(&fn->cfunc_head, child, cfunc_list)
        child->id = nfuncs++;
    fn->nfuncs = nfuncs;
}
static void cblock_rdfo(cblock_t *block, cfunction_t *fn)
{
    cblock_t *succ;

    block->mark = tick;
    array_foreach_entry(&block->succ, succ)
    {
        if(succ->mark != tick)
            cblock_rdfo(succ, fn);
    }
    list_remove(&block->cblock_list);
    list_add(&fn->cblock_head, &block->cblock_list);
}
void cfunc_rdfo(cfunction_t *fn)
{
    tick++;
    cblock_rdfo(fn->entry, fn);
    cfunc_number(fn);
}
static cresult enforce_set(cnode_t *node, cvar_t *var)
{
    if(node->type == CN_SET && var->is_const
       && ((var->type == GLOBAL_INT && var->intl.set != node)
           || var->type == GLOBAL_EXT))
    {
        c_error("invalid assignment to `const %s`", r_symstr(var->name));
        return FAILED;
    }
    if(node->set.value && var->decl)
        return enforce_decl(node, var->decl, &node->set.value, true);
    return SUCCESS;
}
static cresult cfunc_enforce_set(cfunction_t *fn)
{
    cblock_t *block;
    cnode_t *node;
    cresult res = SUCCESS;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
        list_foreach_entry(&block->cnode_head, node, cnode_list)
            if(node->type == CN_SET || node->type == CN_BIND)
                res |= enforce_set(node, node->set.var);
    return res;
}
cresult ir_prepare(cfunction_t *fn)
{
    cresult res;

    cfunc_cleanup(fn);
    cfunc_crit_edges(fn);
    cfunc_node_users(fn);
    res = cfunc_enforce_set(fn);
    cfunc_rdfo(fn);
    res |= cfunc_map_children(fn, ir_prepare);
    cfunc_init_closure(fn);
    return res;
}
