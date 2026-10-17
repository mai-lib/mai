#include "global.h"
#include "ir.h"
#include "gen.h"
// for gen_code.h
#include "vm/vm_ops.h"
// for the globals and loc_for/rec_for
#include "gen_code.h"
static gen_copy_t *make_copy(op_stack_t dest, op_stack_t src, rtype_t *type)
{
    gen_copy_t *copy = xmalloc(sizeof(*copy));

    *copy = (gen_copy_t) {
        .dest = dest,
        .src = src,
        .type = type
    };
    return copy;
}
typedef struct {
    op_stack_t el, loc;
    gen_copy_t *pred;
} mapelt_t;
typedef struct {
    int nelts;
    mapelt_t *elts;
} schedmap_t;
static inline mapelt_t *map_get(schedmap_t *map, op_stack_t el)
{
    for(int i = 0; i < map->nelts; i++)
    {
        mapelt_t *elt = &map->elts[i];
        if(elt->el == el)
            return elt;
    }
    return NULL;
}
static inline void map_init(schedmap_t *map, int i, op_stack_t el)
{
    map->elts[i].el = el;
    map->elts[i].loc = el;
}
static inline void schedule_init(list_t *in, op_stack_t temploc, schedmap_t *map,
                                 list_t *ready, list_t *todo)
{
    gen_copy_t *copy, *tmp;
    int i = 0;

    list_foreach_entry(in, copy, list)
        map_init(map, i++, copy->src);
    map_init(map, i, temploc);

    list_foreach_entry_safe(in, copy, tmp, list)
    {
        mapelt_t *elt = map_get(map, copy->dest);

        if(elt)
        {
            elt->pred = copy;
            list_remove(&copy->list);
            list_add(todo, &copy->list);
        }
        else
        {
            list_remove(&copy->list);
            list_add(ready, &copy->list);
        }
    }
}
static void schedule_ready(gen_copy_t *copy, list_t *out, schedmap_t *map,
                           list_t *ready)
{
    mapelt_t *elt = map_get(map, copy->src);

    list_remove(&copy->list);
    list_add_before(out, &copy->list);
    if(copy->src == elt->loc && elt->pred)
    {
        list_remove(&elt->pred->list);
        list_add(ready, &elt->pred->list);
    }
    copy->src = elt->loc;
    elt->loc = copy->dest;
}
static void schedule_todo(gen_copy_t *copy, op_offset_t temploc, list_t *out,
                          schedmap_t *map, list_t *ready)
{
    mapelt_t *elt = map_get(map, copy->src);
    gen_copy_t *tempcopy = make_copy(temploc, copy->src, copy->type);

    elt->loc = temploc;
    list_add_before(out, &tempcopy->list);
    list_remove(&elt->pred->list);
    list_add(ready, &elt->pred->list);
    list_remove(&copy->list);
}
static void schedule_copies(op_offset_t temploc, list_t *in, unsigned ncopies,
                            list_t *out)
{
    list_t ready = LIST_INIT(ready), todo = LIST_INIT(todo);
    schedmap_t map = {
        .nelts = ncopies + 1,
        .elts = xcalloc(ncopies + 1, sizeof(mapelt_t))
    };

    schedule_init(in, temploc, &map, &ready, &todo);
    while(!list_isempty(&ready) || !list_isempty(&todo))
    {
        while(!list_isempty(&ready))
            schedule_ready(container_of(ready.next, gen_copy_t, list),
                           out, &map, &ready);
        if(!list_isempty(&todo))
            schedule_todo(container_of(todo.next, gen_copy_t, list),
                          temploc, out, &map, &ready);
    }
    xfree(map.elts);
}
void gen_un_ssa(cfunction_t *fn, op_offset_t temploc)
{
    cblock_t *block;
    list_t copies = LIST_INIT(copies);

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        gen_block_t *rec = rec_for(block);
        int i, ncopies = 0;

        rec->copies = (list_t) LIST_INIT(rec->copies);
        array_foreach(&block->succ, i)
        {
            cblock_t *succ = aref(&block->succ, i);
            int j = index_of_block(&succ->pred, block);
            cnode_t *node;

            list_foreach_entry(&succ->cnode_head, node, cnode_list)
            {
                if(node->type != CN_PHI)
                    break;
                cnode_t *arg = aref(&node->phi.args, j);
                op_stack_t dest = loc_for(node), src = loc_for(arg);

                if(src != dest)
                {
                    gen_copy_t *copy = make_copy(dest, src, decl_type(node->decl));

                    list_add(&copies, &copy->list);
                    ncopies++;
                }
            }
        }
        if(ncopies > 0)
            schedule_copies(temploc, &copies, ncopies, &rec->copies);
    }
}
