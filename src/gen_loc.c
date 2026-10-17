#include "global.h"
#include "ir.h"
// for vm_act_rec_t
#include "vm/vm_ops.h"
#include "gen.h"
#include "gen_range.h"
static void args_assign(cfunction_t *fn, loc_t *locs)
{
    int argi = 0;
    cblock_t *block = NULL;
    cnode_t *node;

    block = list_entry(fn->entry, block, cblock_list);
    list_foreach_entry(&block->cnode_head, node, cnode_list)
    {
        if(node->type != CN_BIND)
            continue;
        locs[node->id] = (loc_t) {
            .base = BASE_ARG,
            .index = argi++
        };
    }
}
static int cmp_node_id(const void *a, const void *b)
{
    cnode_t *x = *(cnode_t **)a, *y = *(cnode_t **)b;
    return x->id - y->id;
}
static void phi_assign_hints(loc_t *locs, cnode_t *phi)
{
    cnode_array_t arr = ARRAY_INIT;
    cnode_t *node, *tail = NULL;

    array_copy(&arr, &phi->phi.args);
    array_push(&arr, phi);
    qsort(arr.ptr, arr.length, sizeof(cnode_t *), cmp_node_id);
    array_foreach_entry(&arr, node)
    {
        loc_t *loc = &locs[node->id];

        if(loc->base != BASE_NONE)
        {
            if(loc->base == BASE_HINT)
                tail = node;
        }
        else if(!tail)
            tail = node;
        else if(node != tail)
        {
            *loc = (loc_t) {
                .base = BASE_HINT,
                .index = tail->id
            };
            tail = node;
        }
    }
    array_fini(&arr);
}
static void phis_assign(cfunction_t *fn, liveranges_t *live, loc_t *locs)
{
    range_t *range;

    list_foreach_entry(&live->range_head, range, range_list)
        if(range->node->type == CN_PHI)
            phi_assign_hints(locs, range->node);
}
typedef struct
{
    list_t active, inactive;
    ARRAY(bool) used;
} local_t;
static unsigned find_free_index(local_t *local, loc_t *locs, loc_t *loc)
{
    unsigned i;

    if(loc->base == BASE_HINT)
    {
        loc_t *other = &locs[loc->index];

        if(!aref(&local->used, other->index))
            return other->index;
    }
    array_foreach(&local->used, i)
        if(!aref(&local->used, i))
            return i;
    array_extend(&local->used, 1);
    return i;
}
void loc_alloc(cfunction_t *fn, liveranges_t *live, loc_t *locs, unsigned nlocs[])
{
    local_t locals[LOCAL_BASES];
    list_t *unhandled = &live->range_head;
    range_t *cur;

    for(int i = 0; i < LOCAL_BASES; i++)
    {
        local_t *local = &locals[i];

        *local = (local_t) {
            .active = LIST_INIT(local->active),
            .inactive = LIST_INIT(local->inactive),
            .used = ARRAY_INIT
        };
    }
    list_while_entry(unhandled, cur, range_list)
    {
        range_t *range, *tmp;
        locbase base = base_for(cur->node);
        local_t *local = &locals[base];
        loc_t *loc = &locs[cur->node->id];

        list_remove(&cur->range_list);
        if(loc->base < UNASSIGNED)
            continue;
        list_foreach_entry_safe(&local->active, range, tmp, range_list)
        {
            if(range->end <= cur->start)
            {
                list_remove(&range->range_list);
                aset(&local->used, locs[range->node->id].index, false);
            }
            else if(!range_covers(range, cur->start))
            {
                aset(&local->used, locs[range->node->id].index, false);
                list_remove(&range->range_list);
                list_add(&local->inactive, &range->range_list);
            }
        }
        list_foreach_entry_safe(&local->inactive, range, tmp, range_list)
        {
            if(range->end <= cur->start)
                list_remove(&range->range_list);
            else if(range_covers(range, cur->start))
            {
                list_remove(&range->range_list);
                list_add(&local->active, &range->range_list);
                aset(&local->used, locs[range->node->id].index, true);
            }
        }
        unsigned index = find_free_index(local, locs, loc);

        aset(&local->used, index, true);
        *loc = (loc_t) {
            .base = base,
            .index = index
        };
        list_add_before(&local->active, &cur->range_list);
    }
    for(int i = 0; i < LOCAL_BASES; i++)
    {
        local_t *local = &locals[i];

        nlocs[i] = alen(&local->used);
        array_fini(&local->used);
        list_remove(&local->inactive);
        list_remove(&local->active);
    }
}
static void loc_bases(op_offset_t *base_ofs, unsigned nlocs[],
                      op_offset_t *pscalsz, op_offset_t *psz)
{
    size_t sz = 0;

    for(locbase i = BASE_8; i < SCALAR_BASES; i++)
    {
        base_ofs[i] = sz;
        sz += nlocs[i] * size_base(i);
    }
    *pscalsz = sz;
    if(sz > SHRT_MAX)
        fatal("out of local scalar stack space.");
    base_ofs[BASE_PTR] = sz;
    sz += nlocs[BASE_PTR] * size_base(BASE_PTR);
    if(sz > SHRT_MAX)
        fatal("out of local stack space.");
    *psz = sz;
}
static inline op_stack_t arg_offset(cfunction_t *fn, loc_t *loc)
{
    funsig_t *sig = fn->cl_type->sig;

    return -(sig->argsz + sizeof(vm_act_rec_t)) +
            (!loc->index ? 0 : sig->args[loc->index - 1].offset);
}
static inline op_stack_t local_offset(op_offset_t *base_ofs, loc_t *loc)
    { return base_ofs[loc->base] + size_base(loc->base) * loc->index; }
static op_stack_t *loc_offsets(cfunction_t *fn, op_offset_t *base_ofs,
                               loc_t *locs)
{
    op_stack_t *ofs = xcalloc(fn->nnodes, sizeof(*ofs));
    cblock_t *block;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        cnode_t *node;

        list_foreach_entry(&block->cnode_head, node, cnode_list)
        {
            loc_t *loc = &locs[node->id];

            if(!cnode_yields_value(node))
                continue;
            ofs[node->id] = (loc->base == BASE_ARG)
                          ? arg_offset(fn, loc)
                          : local_offset(base_ofs, loc);
        }
    }
    return ofs;
}
static inline loc_t *init_locs(cfunction_t *fn)
{
    loc_t *locs = xcalloc(fn->nnodes, sizeof(*locs));
    for(int i = 0; i < fn->nnodes; i++)
        locs[i].base = BASE_NONE;
    return locs;
}
op_stack_t *gen_locations(cfunction_t *fn, liveranges_t *live,
                          op_offset_t *pscalsz, op_offset_t *psz)
{
    loc_t *locs = init_locs(fn);
    op_offset_t base_ofs[LOCAL_BASES];
    unsigned nlocs[LOCAL_BASES];
    op_stack_t *ofs;

    args_assign(fn, locs);
    phis_assign(fn, live, locs);
    loc_alloc(fn, live, locs, nlocs);
    loc_bases(base_ofs, nlocs, pscalsz, psz);
    ofs = loc_offsets(fn, base_ofs, locs);
    liveranges_free(fn, live);
    xfree(locs);
    return ofs;
}
