#include "global.h"
#include "ir.h"
typedef struct phi
{
    cnode_t *node;
    cvar_t *var;
    enum { PENDING, DEAD, LIVE } state;
    struct phi *next;
} phi_t;
typedef struct
{
    enum { PHI, NODE } type;
    union
    {
        phi_t *phi;
        cnode_t *node;
    };
} defsrc_t;
typedef struct def def_t;
typedef def_t *def_stack_t;
typedef struct def
{
    defsrc_t src;
    cblock_t *block;
    def_stack_t *next_stack;
    def_t *next;
} def_t;
static def_stack_t *defs;
static phi_t **phis;
static cvar_array_t vars;
static void place_phi(cblock_t *block, cvar_t *var)
{
    unsigned len = alen(&block->pred);
    phi_t *phi = xmalloc(sizeof(*phi));
    cnode_t *node = cnode_create(block, CN_PHI);

    *phi = (phi_t) {
        .node = node,
        .var = var,
        .state = PENDING
    };
    array_init(&node->phi.args, len);
    array_resize(&node->phi.args, len);
    slist_push(phis[block->id], phi, next);
}
static void insert_phis(doms_t *dom, cfunction_t *fn)
{
    ARRAY(cblock_t *) work = ARRAY_INIT;
    unsigned *added = xcalloc(fn->nblocks, sizeof(unsigned));
    unsigned *processed = xcalloc(fn->nblocks, sizeof(unsigned));
    unsigned tick = 0;
    cblock_t *block, *wkblk;
    cnode_t *node;
    cvar_t *var;

    array_foreach_entry(&vars, var)
    {
        tick++;
        list_foreach_entry(&fn->cblock_head, block, cblock_list)
        {
            unsigned id = block->id;

            list_foreach_entry(&block->cnode_head, node, cnode_list)
            {
                if((node->type == CN_SET || node->type == CN_BIND)
                   && node->set.var == var)
                {
                    processed[id] = tick;
                    array_push(&work, block);
                    break;
                }
            }
        }
        while(array_take(&work, &wkblk))
        {
            array_foreach_entry(&dom->dfs[wkblk->id], block)
            {
                unsigned id = block->id;

                if(added[id] < tick)
                {
                    place_phi(block, var);
                    added[id] = tick;
                    if(processed[id] < tick)
                    {
                        processed[id] = tick;
                        array_push(&work, block);
                    }
                }
            }
        }
    }
    xfree(added);
    xfree(processed);
    array_fini(&work);
}
static cnode_t *lookup_def(cvar_t *var)
{
    def_t *def = defs[var->id];

    if(!def)
        return NULL;
    if(def->src.type == PHI)
    {
        phi_t *phi = def->src.phi;

        if(phi->state != DEAD)
        {
            phi->state = LIVE;
            return phi->node;
        }
        return NULL;
    }
    return def->src.node;
}
static inline bool has_var(cvar_t *var)
{
    cvar_t *v;

    array_foreach_entry(&vars, v)
        if(v == var)
            return true;
    return false;
}
static def_stack_t *push_def(cvar_t *var, defsrc_t src,
                             cblock_t *block, def_stack_t *unwind)
{
    def_stack_t *stack = &defs[var->id];
    def_t *def = xmalloc(sizeof(*def));

    *def = (def_t) {
        .src = src,
        .block = block,
        .next_stack = unwind
    };
    slist_push(*stack, def, next);
    return stack;
}
static void ssa_convert(doms_t *dom, cblock_t *block)
{
    phi_t *phi;
    cnode_t *node, *tmp;
    cblock_t *nextblk;
    def_stack_t *unwind = NULL;

    slist_foreach(phis[block->id], phi, next)
    {
        defsrc_t src = { .type = PHI, .phi = phi };

        unwind = push_def(phi->var, src, block, unwind);
    }
    list_foreach_entry_safe(&block->cnode_head, node, tmp, cnode_list)
    {
        if((node->type == CN_SET || node->type == CN_BIND)
                && has_var(node->set.var))
        {
            cvar_t *var = node->set.var;
            def_t *tos = defs[var->id];
            defsrc_t src = {
                .type = NODE,
                .node = node->set.value ? node->set.value : node
            };

            if(tos && tos->block == node->block)
                tos->src = src;
            else
                unwind = push_def(var, src, block, unwind);
            if(node->set.value)
                cnode_remove(node);
        }
        else if(node->type == CN_REF && has_var(node->ref.var))
        {
            cnode_t *value = lookup_def(node->ref.var);
            assert(value);
            cnode_replace_in_users(node, value);
            cnode_free(node);
        }
    }
    array_foreach_entry(&block->succ, nextblk)
    {
        int i = index_of_block(&nextblk->pred, block);

        slist_foreach(phis[nextblk->id], phi, next)
        {
            cnode_t *value = lookup_def(phi->var);

            if(value)
                aset(&phi->node->phi.args, i, value);
            else
                phi->state = DEAD;
        }
    }
    array_foreach_entry(&dom->children[block->id], nextblk)
        ssa_convert(dom, nextblk);
    while(unwind)
    {
        def_t *def = *unwind;

        slist_remove_head(*unwind, next);
        unwind = def->next_stack;
        xfree(def);
    }
}
static void finalize_phis(cfunction_t *fn)
{
    cblock_t *block;
    phi_t *phi, *tmp;

    list_foreach_entry(&fn->cblock_head, block, cblock_list)
    {
        slist_foreach_safe(phis[block->id], phi, tmp, next)
        {
            if(phi->state == LIVE)
            {
                cnode_t *arg;

                array_foreach_entry(&phi->node->phi.args, arg)
                    cnode_add_user(phi->node, arg);
                list_add(&block->cnode_head, &phi->node->cnode_list);
            }
            else
                cnode_free(phi->node);
            xfree(phi);
        }
    }
}
static void collect_vars(cfunction_t *fn, cvar_array_t *vars)
{
    int nvars = 0;
    cvar_t *var;

    array_init(vars, 0);
    list_foreach_entry(&fn->cvar_head, var, cvar_list)
    {
        if(cvar_is_global(var) || cvar_is_celled(var))
            continue;
        var->id = nvars++;
        array_push(vars, var);
    }
}
void cfunc_ssa_convert(cfunction_t *fn)
{
    doms_t *dom = cfunc_dom(fn);

    collect_vars(fn, &vars);
    defs = xcalloc(vars.length, sizeof(*defs));
    phis = xcalloc(fn->nblocks, sizeof(*phis));
    insert_phis(dom, fn);
    ssa_convert(dom, fn->entry);
    finalize_phis(fn);
    xfree(phis);
    xfree(defs);
    array_fini(&vars);
    dom_free(dom, fn->nblocks);
    cfunc_cleanup(fn);
    assert(!cfunc_crit_edges(fn));
    cfunc_rdfo(fn);
}
void ir_ssa_convert(cfunction_t *fn)
{
    cfunc_ssa_convert(fn);
    cfunc_mapc_children(fn, ir_ssa_convert);
}
