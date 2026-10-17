#include "global.h"
#include "ir.h"
#include "opt.h"
static cnode_t **VN;
hashset_t *tbl;
static inline void hash_ptr(const void *ptr, uint32_t *phash)
    { *phash = hash_code_seed(ptr, sizeof(void *), *phash); }
static uint32_t cnode_hash(const void *ptr)
{
    int i;
    const cnode_t *node = ptr;
    uint32_t hash = hash_code(&node->type, sizeof(cnodetype));

    hash_ptr(&node->decl, &hash);
    switch(node->type)
    {
    case CN_CONST: hash_ptr(&node->constant, &hash); break;
    case CN_IF: hash_ptr(&node->ifelse.cond, &hash); break;
    case CN_RETURN: hash_ptr(&node->ret.value, &hash); break;
    case CN_COPY: hash_ptr(&node->copy.value, &hash); break;
    case CN_REF: hash_ptr(&node->ref.var, &hash); break;
    case CN_LAMBDA: hash_ptr(&node->lambda.function, &hash); break;
    case CN_CALL:
    case CN_CALL_FAST:
        hash_ptr(&node->call.target, &hash);
        array_foreach(&node->call.args, i)
        {
            hash_ptr(aptr(&node->call.args, i), &hash);
            if(node->type == CN_CALL && alen(&node->call.names) > 0)
                hash_ptr(aptr(&node->call.names, i), &hash);
        }
        if(node->type == CN_CALL_FAST)
            hash = hash_code_seed(&node->call.argbits,
                                  sizeof(argbits_t), hash);
        break;
    case CN_BIND:
    case CN_SET:
        hash_ptr(&node->set.var, &hash);
        hash_ptr(&node->set.value, &hash);
        break;
    case CN_PHI:
        array_foreach(&node->phi.args, i)
            hash_ptr(aptr(&node->phi.args, i), &hash);
        break;
    case CN_BUILTIN:
        hash_ptr(&node->builtin.bi, &hash);
        hash_ptr(&node->builtin.optype, &hash);
        array_foreach(&node->builtin.args, i)
            hash_ptr(aptr(&node->builtin.args, i), &hash);
        break;
    }
    return hash;
}
static bool cnode_equal(const void *x, const void *y)
{
    const cnode_t *nx = x, *ny = y;
    int i;

    if(nx == ny)
        return true;
    if(nx->type != ny->type || nx->decl != ny->decl)
        return false;
    switch(nx->type)
    {
    case CN_CONST: return nx->constant == ny->constant;
    case CN_IF: return nx->ifelse.cond == ny->ifelse.cond;
    case CN_RETURN: return nx->ret.value == ny->ret.value;
    case CN_COPY: return nx->copy.value == ny->copy.value;
    case CN_REF: return nx->ref.var == ny->ref.var;
    case CN_LAMBDA: return nx->lambda.function == ny->lambda.function;
    case CN_CALL:
    case CN_CALL_FAST:
        if(nx->call.target != ny->call.target ||
           alen(&nx->call.args) != alen(&ny->call.args))
            return false;
        array_foreach(&nx->call.args, i)
        {
            if(aref(&nx->call.args, i) != aref(&ny->call.args, i))
                return false;
            if(nx->type == CN_CALL && alen(&nx->call.names) > 0
               && aref(&nx->call.names, i) != aref(&ny->call.names, i))
                return false;
        }
        if(nx->type == CN_CALL_FAST
           && nx->call.argbits != ny->call.argbits)
            return false;
        break;
    case CN_BIND:
    case CN_SET:
        return nx->set.var == ny->set.var && nx->set.value == ny->set.value;
    case CN_PHI:
        if(alen(&nx->phi.args) != alen(&nx->phi.args))
            return false;
        array_foreach(&nx->phi.args, i)
            if(aref(&nx->phi.args, i) != aref(&ny->phi.args, i))
                return false;
        break;
    case CN_BUILTIN:
        if(nx->builtin.bi != ny->builtin.bi
           || nx->builtin.optype != ny->builtin.optype
           || alen(&nx->builtin.args) != alen(&ny->builtin.args))
            return false;
        array_foreach(&nx->builtin.args, i)
            if(aref(&nx->builtin.args, i) != aref(&ny->builtin.args, i))
                return false;
        break;
    }
    return true;
}
static void lookup_vn(cnode_t **ptr, void *data)
{
    assert(*ptr);
    cnode_remove_user(data, *ptr);
    assert(VN[(*ptr)->id]);
    *ptr = VN[(*ptr)->id];
    cnode_add_user(data, *ptr);
}
static inline void lookup_operands(cnode_t *node)
{
    cnode_map_used(node, lookup_vn, node);
}
static inline bool should_record_node(cnode_t *node)
{
    if(node->type == CN_REF && cvar_is_global(node->ref.var) &&
       !node->ref.var->is_const)
        return false;
    return cnode_is_pure(node, true);
}
static bool phi_args_valid(cnode_t *node)
{
    cnode_t *arg;

    array_foreach_entry(&node->phi.args, arg)
        if(!VN[arg->id])
            return false;
    return true;
}
static inline cnode_t *record_node(cnode_t *node, cnode_array_t *scope)
{
    cnode_t *val = hashset_insert(tbl, node);

    if(val != node)
    {
        cnode_replace_in_users(node, val);
        cnode_remove(node);
        return val;
    }
    array_push(scope, node);
    return node;
}
static void dvn(doms_t *dom, cblock_t *block)
{
    cblock_t *nextblk;
    cnode_t *node, *tmp;
    cnode_array_t scope = ARRAY_INIT;

    list_foreach_entry_safe(&block->cnode_head, node, tmp, cnode_list)
    {
        unsigned id = node->id;

        if(node->type != CN_PHI || phi_args_valid(node))
            lookup_operands(node);
        VN[id] = should_record_node(node)
               ? record_node(node, &scope)
               : node;
    }
    array_foreach_entry(&block->succ, nextblk)
    {
        int i = index_of_block(&nextblk->pred, block);

        list_foreach_entry(&nextblk->cnode_head, node, cnode_list)
        {
            if(node->type != CN_PHI)
                break;
            lookup_vn(aptr(&node->phi.args, i), node);
        }
    }
    array_foreach_entry(&dom->children[block->id], nextblk)
        dvn(dom, nextblk);
    array_foreach_entry(&scope, node)
        hashset_remove(tbl, node);
    array_fini(&scope);
}
static void cfunc_dvn(cfunction_t *fn)
{
    doms_t *dom = cfunc_dom(fn);

    tbl = hashset_create(cnode_hash, cnode_equal);
    VN = xcalloc(fn->nnodes, sizeof(*VN));
    dvn(dom, fn->entry);
    xfree(VN);
    hashset_free(tbl);
    dom_free(dom, fn->nblocks);
    cfunc_cleanup(fn);
    cfunc_rdfo(fn);
}
void ir_dvn(cfunction_t *fn)
{
    if(!opt.opt_dvn)
        return;
    cfunc_dvn(fn);
    cfunc_mapc_children(fn, ir_dvn);
}
