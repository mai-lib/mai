#include "global.h"
#include "ir.h"
#include "gen.h"
#include "gen_range.h"
static inline bool interval_covers(interval_t *it, unsigned point)
    { return it->start <= point && point < it->end; }
static interval_t *interval_create_after(range_t *range, interval_t *prev,
                                         unsigned point)
{
    interval_t *it = xmalloc(sizeof(*it));

    it->start = it->end = point;
    list_init(&it->it_list);
    list_add(&prev->it_list, &it->it_list);
    return it;
}
static interval_t *interval_add(range_t *range, unsigned point)
{
    interval_t *prev;

    list_foreach_entry_reverse(&range->it_head, prev, it_list)
    {
        if(prev->end < point)
            break;
        else if(prev->start <= point)
            return prev;
    }
    return interval_create_after(range, prev, point);
}
static interval_t *interval_extend(range_t *range, interval_t *it,
                                   unsigned point)
{
    if(it->it_list.prev != &range->it_head)
    {
        interval_t *prev = list_entry(it->it_list.prev, it, it_list);

        if(prev->end >= point)
        {
            assert(prev->start <= point);
            prev->end = it->end;
            list_remove(&it->it_list);
            xfree(it);
            return prev;
        }
    }
    if(it->start > point)
        it->start = point;
    return it;
}
bool range_covers(range_t *range, unsigned point)
{
    interval_t *it;

    list_foreach_entry(&range->it_head, it, it_list)
    {
        if(interval_covers(it, point))
            return true;
        if(it->start > point)
            return false;
    }
    return false;
}
static inline cblock_t *skip_empty(cblock_t *block)
{
    if(list_isempty(&block->cnode_head))
        return aref(&block->pred, 0);
    return block;
}
static void range_extend(range_t *range, cblock_t *block, cnode_t *node,
                         interval_t *it)
{
    unsigned point;
    cblock_t *pred;

    if(node->block == block && node->type != CN_PHI)
        point = node->id;
    else
        point = block->start;
    if(interval_covers(it, point))
        return;
    interval_extend(range, it, point);
    if(node->block == block)
        return;
    array_foreach_entry(&block->pred, pred)
    {
        pred = skip_empty(pred);
        it = interval_add(range, pred->end + 1);
        range_extend(range, pred, node, it);
    }
}
static void range_free(range_t *range)
{
    interval_t *it, *itmp;

    list_foreach_entry_safe(&range->it_head, it, itmp, it_list)
        xfree(it);
    //list_init(&range->it_head);
    //list_remove(&range->range_list);
}
static void range_extents(range_t *range)
{
    interval_t *first = NULL, *last = NULL;

    assert(!list_isempty(&range->it_head));
    first = list_entry(range->it_head.next, first, it_list);
    last = list_entry(range->it_head.prev, last, it_list);
    range->start = first->start;
    range->end = last->end;
    assert(range->start <= range->end);
}
static void liveranges_compute(cfunction_t *fn, liveranges_t *live)
{
    cblock_t *block;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        cnode_t *node;

        list_foreach_entry(&block->cnode_head, node, cnode_list)
        {
            range_t *range = range_for(live, node);
            cnode_t *user;

            list_init(&range->it_head);
            list_init(&range->range_list);
            if(!cnode_yields_value(node))
                continue;
            range->node = node;
            array_foreach_entry(&node->users, user)
            {
                cblock_t *begin;
                interval_t *it;

                if(user->type == CN_PHI)
                {
                    int i = index_of_node(&user->phi.args, node);
                    assert(i >= 0);
                    begin = skip_empty(aref(&user->block->pred, i));
                    it = interval_add(range, begin->end + 1);
                }
                else
                {
                    begin = user->block;
                    it = interval_add(range, user->id);
                }
                range_extend(range, begin, node, it);
            }
            if(!cnode_is_used(node))
                interval_add(range, node->id);
            range_extents(range);
            list_add_before(&live->range_head, &range->range_list);
        }
    }
}
void liveranges_free(cfunction_t *fn, liveranges_t *live)
{
    range_t *range;

    for(int i = 0; i < fn->nnodes; i++)
    {
        range = &live->ranges[i];

        if(!list_isempty(&range->it_head))
            range_free(range);
    }
    xfree(live->ranges);
    xfree(live);
}
liveranges_t *cfunc_liveranges(cfunction_t *fn)
{
    liveranges_t *live = xmalloc(sizeof(*live));

    *live = (liveranges_t) {
        .ranges = xcalloc(fn->nnodes, sizeof(range_t))
    };
    list_init(&live->range_head);
    liveranges_compute(fn, live);
    return live;
}
