#include "global.h"
#include "ir.h"
#include "opt.h"
#include "vm/vm_ops.h"
#include "gen_code.h"
#include "arith/scalar.h"
#include "arith/vec.h"
#include "arith/dispatch.h"

static void gen_arith(cnode_t *node)
{
    const arith_builtin_t *ab = node->builtin.bi->data;
    assert(ab->op);
    // OP.type dest, args...
    asm_op_type(ab->op, node->builtin.optype);
    asm_stack(loc_for(node));
    asm_args(&node->builtin.args);
}

static void gen_generic(cnode_t *node)
{
    const arith_builtin_t *ab = node->builtin.bi->data;
    assert(ab->op);
    // OP dest, args...
    asm_op(ab->op);
    asm_stack(loc_for(node));
    asm_args(&node->builtin.args);
}

#if 0
// XXX unused!
static void gen_width(cnode_t *node)
{
    assert(!"handled");
    op_code_t op = (op_code_t)(intptr_t)node->builtin.bi->data;

    // OP.width dest, obj, idx
    asm_op_width(op, node->builtin.optype);
    asm_stack(loc_for(node));
    asm_args(&node->builtin.args);
}
#endif

// arith
static inline cresult
fold_binary(binop_chk_t *chk, binop_fn fn, cell_t *cell, cell_t *xc, cell_t *yc)
{
    // constant values available?
    if(opt.opt_constfold && fn && cell_const_obj(xc) && cell_const_obj(yc))
    {
        robject_t *xv = cell_const(xc), *yv = cell_const(yc);
        rvalue_union_t val;
        robject_t *obj;

        // fold operation on constants
        fn(&val, BOXPTR(xv), BOXPTR(yv), 1);
        obj = c_intern(r_box(chk->typ, &val));
        cell_set_const(cell, obj); // sets type
        return CHANGED;
    }
    return SUCCESS;
}

static cresult trans_binary(opt_phase phase, cnode_t *node, cell_t *cell,
                            const cbuiltin_t *bi, cnode_array_t *args)
{
    const arith_builtin_t *ab = bi->data;

    if(!check_nargs(2, node, args))
        return SUCCESS;

    cell_t *xc = cell_for(aref(args, 0));
    cell_t *yc = cell_for(aref(args, 1));
    rtype_t *xt = cell_type(xc), *yt = cell_type(yc);

    // operand types available?
    if(!xt || !yt)
        return SUCCESS;

    binop_chk_t chk = { 0 };

    // operation is valid on these types?
    if(!check_binary(&chk, xt, yt, ab->is_pred))
    {
        // check_ requires precise types; this call may not be
        // statically invalid, but we can't tell, so punt to runtime
        return SUCCESS;
    }

    // transfer the return type if required
    if(phase == TRANSFER)
        cell_set_type(cell, chk.typ);

    // scalars only
    // XXX allow constant-folding on nonscalars, but never become_builtin
    if(chk.xvec || chk.yvec)
        return SUCCESS;

    binop_fn fn = dispatch_binary(&chk, ab->fntab);

    if(phase == TRANSFER)
    {
        // constant values available?
        return fold_binary(&chk, fn, cell, xc, yc);
    }
    if(!fn) // else phase == TRANSFORM
    {
        c_error("operation `%s` not supported on `%s` and `%s`.",
                bi->name, rtype_name(xt), rtype_name(yt));
        return FAILED;
    }
    if(ab->op)
    {
        // lower the CALL if possible
        call_become_builtin(node, bi, chk.optyp, chk.typ);
        return CHANGED;
    }
    return SUCCESS;
}

static cresult trans_unary(opt_phase phase, cnode_t *node, cell_t *cell,
                           const cbuiltin_t *bi, cnode_array_t *args)
{
    const arith_builtin_t *ab = bi->data;

    if(!check_nargs(1, node, args))
        return SUCCESS;

    cell_t *xc = cell_for(aref(args, 0));
    rtype_t *xt = cell_type(xc);

    if(!xt)
        return SUCCESS;

    unop_chk_t chk = { 0 };

    if(!check_unary(&chk, xt, ab->is_pred))
        return SUCCESS;
    if(phase == TRANSFER)
        cell_set_type(cell, chk.typ);
    if(chk.vec)
        return SUCCESS;

    unop_fn fn = dispatch_unary(&chk, ab->fntab);

    if(phase == TRANSFER)
    {
        if(opt.opt_constfold && fn && cell_const_obj(xc))
        {
            robject_t *xv = cell_const(xc);
            rvalue_union_t val;
            robject_t *obj;

            fn(&val, BOXPTR(xv), 1);
            obj = c_intern(r_box(chk.typ, &val));
            cell_set_const(cell, obj);
            return CHANGED;
        }
        return SUCCESS;
    }
    if(!fn) // else phase == TRANSFORM
    {
        c_error("operation `%s` not supported on `%s`.",
                bi->name, rtype_name(xt));
        return FAILED;
    }
    if(ab->op)
    {
        assert(!chk.vec);
        call_become_builtin(node, bi, xt, chk.typ); // operation occurs at argument type, for e.g. is_na
        return CHANGED;
    }
    return SUCCESS;
}

static cresult trans_reduce(opt_phase phase, cnode_t *node, cell_t *cell,
                            const cbuiltin_t *bi, cnode_array_t *args)
{
    if(!check_nargs(1, node, args))
        return SUCCESS;

    rtype_t *xt = arg_type(args, 0);

    if(!xt)
        return SUCCESS;

    reduce_chk_t chk = { 0 };

    if(check_reduce(&chk, xt) && phase == TRANSFER)
        cell_set_type(cell, chk.typ);
    return SUCCESS;
}


static cresult enforce_arith(cnode_t *node)
{
    cnode_array_t *args = &node->builtin.args;
    cresult res = SUCCESS;
    int i;

    assert(node->builtin.optype);
    // all args coerced to the given type
    array_foreach(args, i)
    {
        cnode_t **ptr = aptr(args, i);
        res |= enforce_decl(node, node->builtin.optype, ptr, true);
    }
    return res;
}

// if an alias may be created because of this answer, only scalars
// (i.e. lowered to BUILTIN) are safe; otherwise we're fine.
static bool arith_is_pure(cnode_t *node, bool may_alias)
{
    return !may_alias || node->type == CN_BUILTIN;
}

// if an alias may be created because of this answer, the answer is no
static bool cons_is_pure(cnode_t *node, bool may_alias)
{
    return !may_alias;
}

const builtin_ops_t
    binary_ops = { trans_binary, enforce_arith, gen_arith,
                   .is_pure = arith_is_pure },
    unary_ops = { trans_unary, enforce_arith, gen_arith,
                  .is_pure = arith_is_pure },
    binary_boolean_ops = { trans_binary, enforce_arith, gen_generic,
                           .is_pure = arith_is_pure },
    unary_boolean_ops = { trans_unary, enforce_arith, gen_generic,
                          .is_pure = arith_is_pure },
    reduce_ops = { trans_reduce, NULL, NULL };

#if 0
// conv, FIXME
static cresult trans_conv(opt_phase phase, cnode_t *node, cell_t *cell,
                          const cbuiltin_t *bi, cnode_array_t *args)
{
    // return become_copy(node, ...);
    return SUCCESS;
}
#endif
const builtin_ops_t conv_ops = { };

// container fetch and store
static rtype_t *trans_vec_fetch(opt_phase phase, cnode_array_t *args)
{
    rtype_t *vt = arg_type(args, 0), *it = arg_type(args, 1);

    if(!vt || !it)
        return NULL;

    idx_chk_t chk = { 0 };

    if(!check_fetch(&chk, vt, it))
        return NULL;
    if(phase == TRANSFER)
        return chk.typ;
    return (chk.kind == SINGLE) ? chk.typ : NULL;
}

static rtype_t *trans_arr_fetch(opt_phase phase, cnode_array_t *args)
{
    int rank = alen(args) - 1;
    rtype_t *vt = arg_type(args, 0);
    rtype_t **ityps = alloca(rank * sizeof(rtype_t *));

    if(!vt)
        return NULL;
    for(int i=0; i<rank; i++)
    {
        ityps[i] = arg_type(args, i+1);
        if(!ityps[i])
            return NULL;
    }

    arr_chk_t chk = { .kinds = alloca(rank * sizeof(idx_kind)) };

    if(!check_arr_fetch(&chk, vt, ityps, rank))
        return NULL;
    if(phase == TRANSFER)
        return chk.typ;
    return (rank == 2 && chk.rank == 0) ? chk.typ : NULL;
}

static cresult trans_fetch(opt_phase phase, cnode_t *node, cell_t *cell,
                           const cbuiltin_t *bi, cnode_array_t *args)
{
    if(!check_arg_names(node) || alen(args) < 2)
        return SUCCESS;

    rtype_t *typ = (alen(args) == 2)
        ? trans_vec_fetch(phase, args)
        : trans_arr_fetch(phase, args);

    if(typ)
    {
        if(phase == TRANSFER)
            cell_set_type(cell, typ);
        else //(phase == TRANSFORM)
            call_become_builtin(node, bi, typ, typ);
        return CHANGED;
    }
    return SUCCESS;
}

static cresult enforce_fetch(cnode_t *node)
{
#ifndef NDEBUG
    rtype_t *type = node->builtin.optype;
    cnode_array_t *args = &node->builtin.args;

    assert(type);
    assert(cnode_compat(aref(args, 1), r_type_int));
    assert(alen(args) != 3 || cnode_compat(aref(args, 2), r_type_int));
    assert(cnode_compat(aref(args, 0), rvec_type_create(type)) ||
        cnode_compat(aref(args, 0), rarr_type_create(type)));
#endif
    return SUCCESS;
}

static void gen_fetch(cnode_t *node)
{
    bool is_mat = (alen(&node->builtin.args) == 3);
    // OP.type(.uni) dest, obj, idx[, idx]
    if(rtype_is_scalar(node->builtin.optype))
        asm_op_type(is_mat ? OP_getel2 : OP_getelt, node->builtin.optype);
    else
        asm_op(is_mat ? OP_getel2ptr : OP_geteltptr);
    asm_stack(loc_for(node));
    asm_args(&node->builtin.args);
}

static rtype_t *trans_vec_store(opt_phase phase, cnode_array_t *args)
{
    rtype_t *vt = arg_type(args, 0);
    rtype_t *rt = arg_type(args, 1);
    rtype_t *it = arg_type(args, 2); // not an error; see `[=` signature

    if(!vt || !rt || !it)
        return NULL;

    idx_chk_t chk = { 0 };

    if(!check_store(&chk, vt, it, rt))
    {
        //cnode_t *v = aref(args, 0);
        //...
        return NULL;
    }

    // `[=` passes through the replacement value, just like `=`
    if(phase == TRANSFER)
        return rt;
    // optype is the container element type; we will convert arg1 if necessary
    return (chk.kind == SINGLE) ? vt->elt : NULL;
}

static rtype_t *trans_arr_store(opt_phase phase, cnode_array_t *args)
{
    int rank = alen(args) - 2;
    rtype_t *vt = arg_type(args, 0);
    rtype_t *rt = arg_type(args, 1);
    rtype_t **ityps = alloca(rank * sizeof(rtype_t *));

    if(!vt || !rt)
        return NULL;

    for(int i=0; i<rank; i++)
    {
        ityps[i] = arg_type(args, i+2);
        if(!ityps[i])
            return NULL;
    }

    arr_chk_t chk = { .kinds = alloca(rank * sizeof(idx_kind)) };

    if(!check_arr_store(&chk, vt, ityps, rank, rt))
        return NULL;
    if(phase == TRANSFER)
        return rt;
    return (rank == 2 && chk.rank == 0) ? vt->elt : NULL;
}

static cresult trans_store(opt_phase phase, cnode_t *node, cell_t *cell,
                           const cbuiltin_t *bi, cnode_array_t *args)
{
    if(!check_arg_names(node) || alen(args) < 3)
        return SUCCESS;

    rtype_t *typ = (alen(args) == 3)
        ? trans_vec_store(phase, args)
        : trans_arr_store(phase, args);

    if(typ)
    {
        if(phase == TRANSFER)
        {
            cell_set_type(cell, typ);
        }
        else // phase == TRANSFORM
        {
            // the opcode for this builtin does not need a new location
            // allocated, so replace us with the replacement value in
            // our users
            cnode_replace_in_users(node, aref(args, 1));
            // node.decl type not actually used
            call_become_builtin(node, bi, typ, typ);
        }
        return CHANGED;
    }
    return SUCCESS;
}

static cresult enforce_store(cnode_t *node)
{
    cnode_array_t *args = &node->builtin.args;

    assert(node->builtin.optype);
    assert(!cnode_is_used(node));
    assert(cnode_compat(aref(args, 2), r_type_int));
    assert(alen(args) != 4 || cnode_compat(aref(args, 3), r_type_int));
    // convert replacement value to optype
    return enforce_decl(node, node->builtin.optype, aptr(args,1), true);
}

static void gen_store(cnode_t *node)
{
    bool is_mat = (alen(&node->builtin.args) == 4);
    // OP.width obj, rpl, idx[, idx]
    asm_op_width(is_mat ? OP_setel2 : OP_setelt, node->builtin.optype);
    asm_args(&node->builtin.args);
}

const builtin_ops_t fetch_ops = { trans_fetch, enforce_fetch, gen_fetch,
                                  .is_pure = cons_is_pure };
const builtin_ops_t store_ops = { trans_store, enforce_store, gen_store,
                                  .is_void = true };

// container constructors
// FIXME just have the one, and determine what exactly to do based on
// bi->ops or whatever
static cresult trans_vector(opt_phase phase, cnode_t *node, cell_t *cell,
                            const cbuiltin_t *bi, cnode_array_t *args)
{
    if(!check_arg_names(node))
        return SUCCESS;
    assert(alen(args) > 0);

    cell_t *arg = cell_for(aref(args, 0));

    if(cell_const_obj(arg) && phase == TRANSFER)
    {
        robject_t *etyp = cell_const(arg);

        if(r_typeof(etyp) == r_type_type)
        {
            cell_set_type(cell, rvec_type_create((rtype_t *)etyp));
            // although doesn't actually matter, as we are not transforming
            return CHANGED;
        }
    }
    return SUCCESS;
}

static cresult trans_array(opt_phase phase, cnode_t *node, cell_t *cell,
                           const cbuiltin_t *bi, cnode_array_t *args)
{
    if(!check_arg_names(node))
        return SUCCESS;
    assert(alen(args) > 0);

    cell_t *arg = cell_for(aref(args, 0));

    if(cell_const_obj(arg) && phase == TRANSFER)
    {
        robject_t *etyp = cell_const(arg);

        if(r_typeof(etyp) == r_type_type)
        {
            cell_set_type(cell, rarr_type_create((rtype_t *)etyp));
            return CHANGED; // although doesn't really matter, as we are not transforming
        }
    }
    return SUCCESS;
}

const builtin_ops_t vector_ops = { trans_vector, NULL, NULL, .is_pure = cons_is_pure };
const builtin_ops_t array_ops = { trans_array, NULL, NULL, .is_pure = cons_is_pure };
