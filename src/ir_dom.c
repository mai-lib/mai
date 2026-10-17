#include "global.h"
#include "ir.h"
static cblock_t *intersect(cblock_t *doms[], cblock_t *b1, cblock_t *b2)
{
    while(b1 != b2)
    {
        while(b1->id > b2->id)
            b1 = doms[b1->id];
        while(b2->id > b1->id)
            b2 = doms[b2->id];
    }
    return b1;
}
static cblock_t **dom_idoms(cfunction_t *fn)
{
    cblock_t **doms = xcalloc(fn->nblocks, sizeof(*doms));
    bool changed = (fn->nblocks > 1);

    doms[0] = fn->entry;
    while(changed)
    {
        cblock_t *block;

        changed = false;
        list_foreach_entry(&fn->cblock_head, block, cblock_list)
        {
            cblock_t *pred, *new_idom = NULL;

            if(block == fn->entry)
                continue;
            array_foreach_entry(&block->pred, pred)
            {
                if(doms[pred->id])
                {
                    if(!new_idom)
                        new_idom = pred;
                    else
                        new_idom = intersect(doms, pred, new_idom);
                }
            }
            if(doms[block->id] != new_idom)
            {
                doms[block->id] = new_idom;
                changed = true;
            }
        }
    }
    return doms;
}
static void add_df(cblock_t *block, cblock_array_t *set)
{
    cblock_t *tmp;

    array_foreach_entry(set, tmp)
        if(tmp == block)
            return;
    array_push(set, block);
}
static doms_t *dom_collect(cfunction_t *fn, cblock_t *doms[])
{
    cblock_array_t *dfs = xcalloc(fn->nblocks, sizeof(*dfs));
    cblock_array_t *children = xcalloc(fn->nblocks, sizeof(*children));
    doms_t *dom = xmalloc(sizeof(*dom));
    cblock_t *block;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        if(block == fn->entry)
            continue;
        array_push(&children[doms[block->id]->id], block);
        if(alen(&block->pred) > 1)
        {
            cblock_t *pred;

            array_foreach_entry(&block->pred, pred)
            {
                cblock_t *runner = pred;
                while(runner != doms[block->id])
                {
                    add_df(block, &dfs[runner->id]);
                    runner = doms[runner->id];
                }
            }
        }
    }
    *dom = (doms_t) {
        .dfs = dfs,
        .children = children,
    };
    return dom;
}
void dom_free(doms_t *dom, int nblocks)
{
    for(int i=0; i<nblocks; i++)
    {
        array_fini(&dom->children[i]);
        array_fini(&dom->dfs[i]);
    }
    xfree(dom->children);
    xfree(dom->dfs);
    xfree(dom);
}
doms_t *cfunc_dom(cfunction_t *fn)
{
    doms_t *dom;
    cblock_t **idom = dom_idoms(fn);

    dom = dom_collect(fn, idom);
    xfree(idom);
    return dom;
}
