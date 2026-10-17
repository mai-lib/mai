#include "global.h"
#include "ir.h"
#include "opt.h"
#include "vm/vm_ops.h"
#include "gen.h"
#include "gen_range.h"
#include "gen_code.h"
void gen_un_ssa(cfunction_t *fn, op_offset_t temploc); // HACK
gen_ctx_t ctx;
static inline rtype_t *type_for(cnode_t *node)
    { return decl_type(node->decl); }
static inline bool is_fallthrough(cblock_t *pred, cblock_t *succ)
    { return pred->cblock_list.next == &succ->cblock_list; }
static void gen_transfer(op_code_t op, cblock_t *to, op_stack_t *extra)
{
    gen_block_t *rec = rec_for(to);

    asm_op(op);
    if(extra)
        asm_stack(*extra);
    asm_offset(rec->addr);
    if(!rec->valid)
    {
        gen_fixup_t *fixup = xmalloc(sizeof(*fixup));

        *fixup = (gen_fixup_t) {
            .addr = asm_len() - sizeof(op_offset_t),
            .next = NULL
        };
        slist_push(rec->fixups, fixup, next);
    }
}
static void gen_jump(cblock_t *from, cblock_t *to)
{
    if(!is_fallthrough(from, to))
        gen_transfer(OP_jump, to, NULL);
}
op_offset_t const_index(void *val)
{
    for(int i = ctx.nfuncs; i < alen(ctx.consts); i++)
        if(aref(ctx.consts, i) == val)
            return i;
    array_push(ctx.consts, val);
    return alen(ctx.consts) - 1;
}
static void gen_literal(op_stack_t dest, void *ptr, rtype_t *type)
{
    asm_op_width(OP_literal, type);
    asm_stack(dest);
    asm_literal(ptr, rtype_eltsz(type));
}
static bool gen_node_const(cnode_t *node)
{
    assert(r_subtypep(r_typeof(node->constant), node->decl));
    if(rtype_is_scalar(node->decl))
    {
        assert(r_typeof(node->constant) == node->decl);
        gen_literal(loc_for(node), BOXPTR(node->constant),
                    node->decl);
    }
    else
    {
        asm_op(OP_const);
        asm_stack(loc_for(node));
        asm_offset(const_index(node->constant));
    }
    return true;
}
static void gen_copy(op_stack_t dest, op_stack_t src,
                     rtype_t *dtype, rtype_t *stype)
{
    if(dtype == stype)
        asm_op_width(OP_mov, stype);
    else if(rtype_is_scalar(dtype) && rtype_is_scalar(stype))
        asm_op_conv(stype, dtype);
    else if(rtype_is_scalar(dtype))
        asm_op_type(OP_unbox, dtype);
    else if(rtype_is_scalar(stype))
        asm_op_type(OP_box, stype);
    else
    {
        asm_op(OP_check);
        asm_stack(src);
        asm_offset(const_index(dtype));
        if(src == dest)
            return;
        asm_op_width(OP_mov, r_type_object);
    }
    asm_stack(dest);
    asm_stack(src);
}
static bool gen_node_copy(cnode_t *node)
{
    cnode_t *val = node->copy.value;

    gen_copy(loc_for(node), loc_for(val),
             type_for(node), type_for(val));
    return true;
}
static bool gen_node_builtin(cnode_t *node)
{
    const cbuiltin_t *bi = node->builtin.bi;
    assert(bi->ops && bi->ops->generate_fn);
    bi->ops->generate_fn(node);
    return true;
}
static bool gen_node_if(cnode_t *node, cblock_t *block)
{
    op_stack_t cond = loc_for(node->ifelse.cond);

    gen_transfer(OP_if, aref(&block->succ, 0), &cond);
    gen_jump(block, aref(&block->succ, 1));
    return false;
}
static bool gen_node_return(cnode_t *node)
{
    asm_op(OP_ret);
    asm_stack(loc_for(node->ret.value));
    return false;
}
static bool gen_node_call_fast(cnode_t *node)
{
    op_stack_t target = loc_for(node->call.target);
    op_stack_t dest = loc_for(node);
    funsig_t *sig = type_for(node->call.target)->sig;
    int i;
    assert(rtype_is_callable(type_for(node->call.target)));
    assert(alen(&node->call.args) == sig->nargs);
    assert((node->call.argbits & sig->reqbits) == sig->reqbits);

    asm_op(OP_frame);
    asm_offset(sig->argsz);
    asm_literal(&node->call.argbits, sizeof(argbits_t));
    array_foreach(&node->call.args, i)
    {
        cnode_t *arg = aref(&node->call.args, i);
        op_stack_t ofs = sig->args[i].offset;

        if(!arg)
            continue;
        gen_copy(ctx.loc_sz + ofs, loc_for(arg),
                 type_for(arg), type_for(arg));
    }
    asm_op(OP_call_fast);
    asm_stack(target);
    asm_op_width(OP_complete, type_for(node));
    asm_stack(dest);
    return true;
}
static bool gen_arg_rest(void *ptr, void *arg)
{
    asm_subop(OP_call_rest);
    asm_stack(loc_for(arg));
    return true;
}
static bool gen_arg_kwd(void *ptr, rsymbol_t *name, void *arg)
{
    op_offset_t idx = const_index(name);

    asm_subop(OP_call_kwd);
    asm_offset(idx);
    asm_stack(loc_for(arg));
    return true;
}
static bool gen_arg_pos(void *ptr, void *arg)
{
    asm_subop(OP_call_pos);
    asm_stack(loc_for(arg));
    return true;
}
static bool gen_arg_omit(void *ptr)
{
    asm_subop(OP_call_omit);
    return true;
}
static bool gen_node_call_universal(cnode_t *node)
{
    op_stack_t target = loc_for(node->call.target);
    op_stack_t dest = loc_for(node);
    cnode_t **args = adata(&node->call.args);
    int nargs = alen(&node->call.args);
    rsymbol_t **names = call_has_names(node)
                      ? adata(&node->call.names)
                      : NULL;

    asm_op(OP_call_uni);
    asm_stack(target);
    call_sequence(NULL, (void **)args, names, nargs,
                  gen_arg_rest, gen_arg_kwd, gen_arg_pos, gen_arg_omit);
    asm_subop(OP_call_end);
    asm_op(OP_complete_box);
    asm_stack(target);
    asm_stack(dest);
    return true;
}
static void gen_closure(cnode_t *node, cfunction_t *fn)
{
    for(int j = 0; j <= 1; j++)
    {
        int i;

        array_foreach(&fn->closure, i)
        {
            cnode_t *val = aref(&node->lambda.closure, i);
            assert(val);
            rtype_t *typ = type_for(val);

            if(j == rtype_is_scalar(typ))
                continue;
            asm_subop(OP_lambda_mov8 + width_code(typ));
            asm_stack(loc_for(val));
        }
    }
    asm_subop(OP_lambda_end);
}
static bool gen_node_lambda(cnode_t *node)
{
    cfunction_t *fn = node->lambda.function;
    bool clo = cfunc_has_closure(fn);
    int id = fn->id;
    assert(alen(&node->lambda.closure) == alen(&fn->closure));
    asm_op(clo ? OP_lambda : OP_const);
    asm_stack(loc_for(node));
    asm_offset(id);
    if(clo)
        gen_closure(node, fn);
    return true;
}
static void gen_setvar(cvar_t *var, cnode_t *val,
                       bool has_decl, bool is_def)
{
    if(has_decl)
        asm_op_width(OP_setvar, decl_type(var->decl));
    else
        asm_op(OP_setvar_uni);
    asm_stack(loc_for(val));
    asm_offset(const_index(var->name));
    if(has_decl)
        asm_offset(is_def);
}
static bool gen_node_set(cnode_t *node)
{
    cvar_t *var = node->set.var;
    assert(r_subtypep(decl_type(node->set.value->decl), decl_type(var->decl)));
    switch(var->type)
    {
    case GLOBAL_INT:
    {
        bool is_def = var->intl.set == node;

        if(is_def)
        {
            asm_op(OP_defvar);
            asm_offset(const_index(var->name));
            asm_offset(const_index(decl_type(var->decl)));
        }
        gen_setvar(var, node->set.value, true, is_def & var->is_const);
        break;
    }
    case GLOBAL_EXT:
    {
        bool has_decl = var->extl.global ? (assert(var->extl.global->decl),true) : false;
        gen_setvar(var, node->set.value, has_decl, false);
        break;
    }
    default: assert(!"reached"); break; /* NOTREACHED */
    }
    return true;
}
static void gen_getvar(cvar_t *var, cnode_t *node, bool has_decl)
{
    if(has_decl)
        asm_op_width(OP_getvar, type_for(node));
    else
        asm_op(OP_getvar_uni);
    asm_stack(loc_for(node));
    asm_offset(const_index(var->name));
}
static inline op_offset_t env_offset(cvar_t *var)
    { return ctx.env_ofs[index_of_var(ctx.closure, var)]; }
static bool gen_node_ref(cnode_t *node)
{
    cvar_t *var = node->ref.var;

    switch(var->type)
    {
    case GLOBAL_INT:
        gen_getvar(var, node, true);
        break;
    case GLOBAL_EXT:
        gen_getvar(var, node, var->extl.global);
        break;
    default:
        asm_op_width(OP_env, type_for(node));
        asm_stack(loc_for(node));
        asm_offset(env_offset(var));
        break;
    }
    return true;
}
static bool gen_node(cblock_t *block, cnode_t *node)
{
    switch(node->type)
    {
    case CN_IF: return gen_node_if(node, block);
    case CN_RETURN: return gen_node_return(node);
    case CN_CALL: return gen_node_call_universal(node);
    case CN_CALL_FAST: return gen_node_call_fast(node);
    case CN_LAMBDA: return gen_node_lambda(node);
    case CN_CONST: return gen_node_const(node);
    case CN_SET: return gen_node_set(node);
    case CN_REF: return gen_node_ref(node);
    case CN_COPY: return gen_node_copy(node);
    case CN_BUILTIN: return gen_node_builtin(node);
    default: return true;
    } /* NOTREACHED */
}
static void gen_copies(gen_block_t *rec)
{
    gen_copy_t *copy;

    list_while_entry(&rec->copies, copy, list)
    {
        list_remove(&copy->list);
        gen_copy(copy->dest, copy->src, copy->type, copy->type);
        xfree(copy);
    }
}
static void gen_fixups(gen_block_t *rec)
{
    gen_fixup_t *fixup;

    slist_while_pop(rec->fixups, fixup, next)
    {
        asm_patch(fixup->addr, rec->addr);
        xfree(fixup);
    }
}
static void gen_block(cblock_t *block)
{
    op_offset_t addr = asm_len();
    gen_block_t *rec = rec_for(block);
    bool fall = true;
    cnode_t *node;

    rec->addr = addr;
    rec->valid = true;
    gen_fixups(rec);
    list_foreach_entry(&block->cnode_head, node, cnode_list)
    {
        assert(fall);
        fall = gen_node(block, node);
    }
    // no critical edges; copies after IF can't happen
    assert(fall || list_isempty(&rec->copies));
    gen_copies(rec);
    if(fall)
    {
        assert(alen(&block->succ) == 1);
        gen_jump(block, aref(&block->succ, 0));
    }
}
static op_code_t *gen_code(cfunction_t *fn, op_stack_t *locs,
                           op_offset_t loc_sz, op_offset_t *env_ofs,
                           robject_array_t *consts)
{
    cblock_t *block;

    array_resize(consts, fn->nfuncs);
    ctx = (gen_ctx_t) {
        .code = ARRAY_INIT,
        .locs = locs,
        .records = xcalloc(fn->nblocks, sizeof(gen_block_t)),
        .consts = consts,
        .nfuncs = fn->nfuncs,
        .closure = &fn->closure,
        .loc_sz = loc_sz,
        .env_ofs = env_ofs
    };
    gen_un_ssa(fn, loc_sz);
    list_foreach_entry(&fn->cblock_head, block, cblock_list)
        gen_block(block);
    xfree(ctx.records);
    xfree(locs);
    if(env_ofs)
        xfree(env_ofs);

    if(opt.dbg_dump_ir)
        asm_dump(fn, &ctx.code, consts);

    return array_cede(&ctx.code);
}
static op_offset_t *closure_env(cfunction_t *fn, op_offset_t *pscalsz,
                                op_offset_t *psz)
{
    op_offset_t *ofs = xcalloc(alen(&fn->closure), sizeof(*ofs));
    op_offset_t sz = 0;

    for(int j = 0; j <= 1; j++)
    {
        int i;

        array_foreach(&fn->closure, i)
        {
            cnode_t *val = aref(&fn->node->lambda.closure, i);
            rtype_t *typ = type_for(val);

            if(j == rtype_is_scalar(typ))
                continue;
            ofs[i] = sz;
            sz += rtype_eltsz(typ);
        }
        if(j == 0)
            *pscalsz = sz;
        else
            *psz = sz;
    }
    return ofs;
}
rfunction_t *gen_function(cfunction_t *fn);
static robject_t *gen_child_function(cfunction_t *child)
{
    rfunction_t *rfn = gen_function(child);

    if(cfunc_has_closure(child))
        return (robject_t *)rfn;
    return (robject_t *)rcall_closure_create(rfn->cl_type, rfn);
}
rfunction_t *gen_function(cfunction_t *fn)
{
    liveranges_t *live = cfunc_liveranges(fn);
    op_offset_t loc_scalsz = 0, loc_sz = 0;
    op_stack_t *locs = gen_locations(fn, live, &loc_scalsz, &loc_sz);
    op_offset_t env_scalsz = 0, env_sz = 0;
    op_offset_t *env_ofs = !cfunc_has_closure(fn) ? NULL
                         : closure_env(fn, &env_scalsz, &env_sz);
    robject_array_t consts = ARRAY_INIT;
    op_code_t *code = gen_code(fn, locs, loc_sz, env_ofs, &consts);
    rfunction_t *rfn = rfunc_create(fn->cl_type, code, loc_scalsz, loc_sz,
                                    env_scalsz, env_sz, &consts);
    cfunction_t *child;

    list_foreach_entry(&fn->cfunc_head, child, cfunc_list)
        rfn->consts[child->id] = gen_child_function(child);
    return rfn;
}
