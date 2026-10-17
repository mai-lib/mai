#include "global.h"
#include "rt/builtin.h"
#include "rt/opt_builtins.h"
#include "ir.h"
#include "opt.h"
#include "vm/vm_ops.h"
#include "gen_code.h"

static cresult trans_typeof(opt_phase phase, cnode_t *node, cell_t *cell,
                            const cbuiltin_t *bi, cnode_array_t *args)
{
    assert(node->type == CN_CALL || node->type == CN_CALL_FAST);
    if(alen(args) != 1)
        return SUCCESS;
    // the condition for typeof could be weakened slightly, to something like
    // "if the static type is the most precise possible dynamic type",
    // but that isn't expressible here.
    if(phase == TRANSFER && opt.opt_constfold)
    {
        cell_t *ac = cell_for(aref(args,0));
        if(!cell_const_obj(ac) && !cell_const_lambda(ac))
                return SUCCESS;

        cell_set_const(cell, (robject_t *)cell_type(ac));
        // DBG("%s folds to ", bi->name);
        // r_print(stderr, cell_type(ac));
        // DBG("\n");
    }
    return SUCCESS;
}

static cresult trans_type(opt_phase phase, cnode_t *node, cell_t *cell,
                          const cbuiltin_t *bi, cnode_array_t *args)
{
    type_op_fn_t *tofn = bi->data;
    assert(node->type == CN_CALL || node->type == CN_CALL_FAST);
    assert(tofn);

    if(alen(args) != tofn->arity)
        return SUCCESS;
    if(phase == TRANSFER && opt.opt_constfold)
    {
        cnode_t *arg;
        array_foreach_entry(args, arg)
        {
            cell_t *ac = cell_for(arg);
            if(!cell_const_obj(ac) || cell_type(ac) != r_type_type)
                return SUCCESS;
        }

        rtype_t *ret = (tofn->arity == 1)
            ? tofn->unfn(cell_const(cell_for(aref(args,0))))
            : tofn->binfn(cell_const(cell_for(aref(args,0))),
                          cell_const(cell_for(aref(args,1))));
        if(ret)
        {
            cell_set_const(cell, (robject_t *)ret);
            // DBG("%s folds to ", bi->name);
            // r_print(stderr, ret);
            // DBG("\n");
        }
    }
    return SUCCESS;
}

// XXX need a user-facing `missing` special form: arg parsed as
// identifier; symbol must name an arg bound by the current function
// convert emits a CN_BUILTIN directly -- like the prologue does in
// ir_convert
static inline cresult fold_missing(cell_t *cell, cell_t *bits, argbits_t idx)
{
    if(opt.opt_constfold && cell_const_obj(bits))
    {
        bool val = argbits_missing(UNBOX(argbits_t, cell_const(bits)), idx);
        cell_set_const(cell, c_intern(r_box(r_type_boolean, &val)));
        return CHANGED;
    }
    return SUCCESS;
}

static cresult trans_missing(opt_phase phase, cnode_t *node, cell_t *cell,
                             const cbuiltin_t *bi, cnode_array_t *args)
{
    assert(node->type == CN_BUILTIN);
    assert(node->builtin.optype && r_typeof(node->builtin.optype) == r_type_int);
    assert(alen(args) == 1);

    cell_t *bits = cell_for(aref(args, 0));
    robject_t *idx = (robject_t *)node->builtin.optype; // this is a hack

    assert(cell_type(bits) == r_type_int);
    if(phase == TRANSFER)
    {
        cell_set_type(cell, r_type_boolean);
        return fold_missing(cell, bits, UNBOX(argbits_t, idx));
    }
    // transform; already a builtin, so don't care
    return SUCCESS;
}

static void gen_missing(cnode_t *node)
{
    // MISSING dest, bits, idx
    asm_op(OP_missing);
    asm_stack(loc_for(node));
    asm_stack(loc_for(aref(&node->builtin.args, 0)));
    // we stashed a box in .optype. retrieve it and emit the value as a literal operand
    assert(node->builtin.optype && r_typeof(node->builtin.optype) == r_type_int);
    asm_offset(UNBOX(argbits_t, node->builtin.optype));
}

static bool always(cnode_t *node, bool may_alias)
    { return true; }

const cbuiltin_t missing = {
    &(builtin_ops_t) { trans_missing, NULL, gen_missing,
                       .is_pure = always },
    NULL, "missing"
};

const builtin_ops_t pure_ops = { .is_pure = always };
const builtin_ops_t typeof_ops = { trans_typeof, NULL, NULL, .is_pure = always };
const builtin_ops_t type_ops = { trans_type, NULL, NULL, .is_pure = always };
