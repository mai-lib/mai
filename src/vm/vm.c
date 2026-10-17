#include "global.h"
#include "arith/scalar.h"
#include "vm/vm_ops.h"
bool scalar_convert(void *dest, robject_t *obj,
                    rtype_t *to, rtype_t *from); // HACK
void vm_error(vm_ctx_t *vm, const char *src, const char *fmt, ...)
{
    va_list va;
    char *tmp;

    va_start(va, fmt);
    vasprintf(&tmp, fmt, va);
    va_end(va);
    asprintf(&vm->err_msg, "%s: %s", src, tmp);
    xfree(tmp);
    siglongjmp(vm->err_buf, -1);
}
static void vm_type_error(vm_ctx_t *vm, const char *src, const char *val,
                          rtype_t *typ, rtype_t *vtyp)
{
    vm_error(vm, src, "%s of type `%s` found where type `%s` expected.",
             val, rtype_name(vtyp), rtype_name(typ));
}
static void mark_locals(uint8_t *bp, rfunction_t *fn)
{
    for(robject_t **obj = (robject_t **)(bp + fn->loc_scalsz);
        obj < (robject_t **)(bp + fn->loc_sz); obj++)
        gc_mark(*obj);
}
static void mark_args(vm_act_rec_t *rec, rcallable_t *fp)
{
    funsig_t *sig = rcall_sig(fp);
    uint8_t *argbase = (uint8_t *)rec - sig->argsz;

    for(int i=0; i < sig->nargs; i++)
    {
        argdesc_t *arg = &sig->args[i];

        if(!rtype_is_scalar(arg->type))
            gc_mark(*(robject_t **)(argbase + arg->offset));
    }
}
static void vm_mark_roots(gc_root_t *root)
{
    vm_ctx_t *vm = container_of(root, vm_ctx_t, gc_root);
    rcallable_t *fp = vm->fp;
    uint8_t *bp = vm->bp;
    robject_t *obj;

    if(!vm->fp)
    {
        gc_mark(vm->ret_val);
        return;
    }
    array_foreach_entry(&vm->retained, obj)
        gc_mark(obj);
    while(bp)
    {
        vm_act_rec_t *rec = (vm_act_rec_t *)bp - 1;

        gc_mark(fp);
        if(!rcall_is_builtin(fp))
            mark_locals(bp, ((rclosure_t *)fp)->fn);
        mark_args(rec, fp);
        bp = rec->bp;
        fp = rec->fp;
    }
}
static inline void
vm_update_ctx(vm_ctx_t *vm, rcallable_t *fp, uint8_t *bp)
{
    vm->fp = fp;
    vm->bp = bp;
}
void *vm_retain(vm_ctx_t *vm, void *obj)
{
    array_push(&vm->retained, obj);
    return obj;
}
void vm_release(vm_ctx_t *vm, int n)
{
    array_drop(&vm->retained, n);
}
#define INIT_RETAINED 16
void vm_init_ctx(vm_ctx_t *vm, uint8_t *stack, size_t stacksz)
{
    *vm = (vm_ctx_t) {
        .stack = stack,
        .stacksz = stacksz,
        .gc_root = { .fn = vm_mark_roots },
        .retained = ARRAY_INIT,
        .err_msg = NULL,
        .fp = NULL
    };
    array_init(&vm->retained, INIT_RETAINED);
    gc_register(&vm->gc_root);
}
void vm_fini_ctx(vm_ctx_t *vm)
{
    array_fini(&vm->retained);
    gc_unregister(&vm->gc_root);
}
#define ADVANCE(n) ((ip += (n)) - (n))
#define FETCH(ctype) (*((ctype *)ADVANCE(sizeof(ctype))))
#define DISPATCH() goto *FETCH(vm_instr_t *);
#define SFETCH() FETCH(op_stack_t)
#define OFETCH() FETCH(op_offset_t)
#define STACK(ctype, ofs) *((ctype *)(bp + ofs))
#define BINOP(op,t) PASTE6(arith_,op,_,t,_,t)
#define UNOP(op,t) PASTE4(arith_,op,_,t)
#define CONVOP(a,b) PASTE4(arith_conv_,a,_to_,b)
#define DO_BINOP(op, arg_t, res_t)                              \
    LABEL(op, arg_t):                                           \
    {                                                           \
        op_stack_t dest = SFETCH(), l = SFETCH(), r = SFETCH(); \
        STACK(R_T(res_t), dest) = BINOP(op,arg_t)               \
            (STACK(R_T(arg_t), l), STACK(R_T(arg_t), r));       \
        DISPATCH();                                             \
    }
#define DO_UNOP(op, arg_t, res_t)                               \
    LABEL(op, arg_t):                                           \
    {                                                           \
        op_stack_t dest = SFETCH(), l = SFETCH();               \
        STACK(R_T(res_t), dest) = UNOP(op,arg_t)                \
            (STACK(R_T(arg_t), l));                             \
        DISPATCH();                                             \
    }
#define DO_CONVOP(a_t, b_t)                                     \
    LABEL(conv, CONV(a_t, b_t)):                                \
    {                                                           \
        op_stack_t dest = SFETCH(), src = SFETCH();             \
        STACK(R_T(b_t), dest) = CONVOP(a_t,b_t)                 \
            (STACK(R_T(a_t), src));                             \
        DISPATCH();                                             \
    }
#define _WIDTH(w) PASTE3(uint, w, _t)
#define op_width_t _WIDTH(op_w)
#define R_T(type) PASTE3(r, type, _t)
#define R_NA(type) PASTE3(r, type, _na)
#define op_type_t R_T(op_t)
typedef struct
{
    rsymbol_t *name;
    robject_t *val;
} kwd_pair_t;
typedef struct
{
    call_ctx_t base;
    uint8_t *sp;
    kwd_pair_t *rp;
    int nrest;
    bool has_names;
    uint8_t *tos;
    vm_ctx_t *vm;
} vm_call_ctx_t;
static bool set_arg(void *ptr, argdesc_t *arg, void *val)
{
    vm_call_ctx_t *ctx = ptr;
    uint8_t *dest = ctx->sp + arg->offset;
    rtype_t *typ = arg->type, *vtyp = r_typeof(val);

    if(rtype_is_scalar(typ))
    {
        if(vtyp == typ)
            r_unbox(dest, val);
        else if(rtype_is_scalar(vtyp))
            scalar_convert(dest, val, typ, vtyp);
        else
            vm_type_error(ctx->vm, "call", "argument",
                          typ, vtyp);
    }
    else
    {
        if(!r_subtypep(vtyp, typ))
            vm_type_error(ctx->vm, "call", "argument",
                          typ, vtyp);
        *(robject_t **)dest = val;
    }
    return true;
}
static bool fail_rest(void *ptr, rsymbol_t *name, void *val)
{
    vm_call_ctx_t *ctx = ptr;

    if(name)
        vm_error(ctx->vm, "call", "unknown argument `%s`.",
                 name->string);
    else
        vm_error(ctx->vm, "call", "too many arguments.");
    return false;
}
static bool append_rest(void *ptr, rsymbol_t *name, void *val)
{
    vm_call_ctx_t *ctx = ptr;

    if((uint8_t *)ctx->rp > ctx->tos)
        vm_error(ctx->vm, "call", "too many arguments; out of stack.");
    if(name)
        ctx->has_names = true;
    *(ctx->rp++) = (kwd_pair_t) {
        .name = name,
        .val = val
    };
    ctx->nrest++;
    return true;
}
static rvector_t *finish_rest(vm_call_ctx_t *ctx)
{
    rvector_t *rest = rvec_create(r_type_vec_object, ctx->nrest);
    robject_t **vals = rvec_elts(rest);
    kwd_pair_t *rp = ctx->rp - ctx->nrest;

    vm_retain(ctx->vm, rest);
    for(int i = 0; i < ctx->nrest; i++)
        vals[i] = rp[i].val;
    if(ctx->has_names)
    {
        rsymbol_t **names = rvec_elts(rvec_add_names(rest));

        for(int i=0; i<ctx->nrest; i++)
            names[i] = rp[i].name;
    }
    vm_release(ctx->vm, 1);
    return rest;
}
static op_code_t *vm_call(vm_ctx_t *vm, funsig_t *sig, op_code_t *ip,
                          rclosure_t *fp, uint8_t *sp, uint8_t *bp)
{
    op_code_t subop;
    vm_call_ctx_t ctx = {
        .base = {
            .posbits = 0,
            .argbits = 0,
            .sig = sig,
            .append_rest = sig->has_rest ? append_rest : fail_rest,
            .set_arg = set_arg
        },
        .sp = sp,
        .rp = (kwd_pair_t *)(sp + sig->argsz),
        .tos = vm->stack + vm->stacksz,
        .nrest = 0,
        .vm = vm
    };

    while((subop = *ip++) != OP_call_end)
    {
        switch(subop)
        {
            case OP_call_kwd:
            {
                op_offset_t idx = OFETCH();
                op_stack_t src = SFETCH();

                rsymbol_t *name = (rsymbol_t *)fp->fn->consts[idx];
                robject_t *val = STACK(robject_t *, src);
                if(!call_match_kwd(&ctx.base, name, val))
                    vm_error(vm, "call", "couldn't pass named argument.");
                break;
            }
            case OP_call_pos:
            {
                op_stack_t src = SFETCH();
                robject_t *val = STACK(robject_t *, src);

                if(!call_match_pos(&ctx.base, val))
                    vm_error(vm, "call", "couldn't pass positional argument.");
                break;
            }
            case OP_call_rest:
            {
                op_stack_t src = SFETCH();
                rvector_t *rest = STACK(rvector_t *, src);

                if(!rest)
                    break;
                if(!r_subtypep(r_typeof(rest), r_type_vec_object))
                    vm_type_error(vm, "call", "rest list", r_type_vec_object,
                                  r_typeof(rest));

                robject_t **vals = rvec_elts(rest);
                rsymbol_t **names = rvec_is_named(rest) ? rvec_elts(rest->names)
                                  : NULL;
                int nvals = rvec_len(rest);
                if(!call_match_rest(&ctx.base, (void **)vals, names, nvals))
                    vm_error(vm, "call", "couldn't pass rest list.");
                break;
            }
            case OP_call_omit:
                if(!call_match_omit(&ctx.base))
                    vm_error(vm, "call", "couldn't pass omitted argument.");
                break;
        }
    }
    if(sig->reqbits != (ctx.base.argbits & sig->reqbits))
        vm_error(vm, "call", "required argument not supplied.");
    *(argbits_t *)sp = ctx.base.argbits;
    if(sig->has_rest && ctx.nrest > 0)
    {
        uint8_t *argp = sp + sig->argsz - sizeof(rvector_t *);
        *(rvector_t **)argp = finish_rest(&ctx);
    }
    return ip;
}
static op_code_t *populate_closure(void *env, op_code_t *ip, uint8_t *bp)
{
    op_code_t subop;
    uint8_t *ptr = env;

    while((subop = *ip++) != OP_lambda_end)
    {
        op_stack_t src = SFETCH();

        switch(subop)
        {
        case OP_lambda_mov8:
            *(uint8_t *)ptr = STACK(uint8_t, src);
            ptr += 1;
            break;
        case OP_lambda_mov32:
            *(uint32_t *)ptr = STACK(uint32_t, src);
            ptr += 4;
            break;
        case OP_lambda_mov64:
            *(uint64_t *)ptr = STACK(uint64_t, src);
            ptr += 8;
            break;
        }
    }
    return ip;
}
static void *init_stack(vm_ctx_t *vm, rclosure_t *fp)
{
    size_t sz = sizeof(argbits_t) + sizeof(vm_act_rec_t);
    uint8_t *bp = (uint8_t *)vm->stack + sz;

    sz += fp->fn->loc_sz;
    memset(vm->stack, 0, sz);
    return bp;
}
const void *const *vm_instr;
int vm_execute(vm_ctx_t *vm, rclosure_t *closure)
{
    #define OP_DEF(def...) [ENUM(def)] = &&LABEL(def)
    static const void *const instr[] =
        {
            OPS_ALL
        };
    #undef OP_DEF
    if(!vm)
    {
        vm_instr = instr;
        return 0;
    }
    op_code_t *ip = closure->fn->code;
    rclosure_t *fp = closure;
    uint8_t *bp = init_stack(vm, closure);
    uint8_t *sp = bp + closure->fn->loc_sz;

    rcallable_t *callee;
    op_offset_t framesize;
    void *pret;

    if(!sigsetjmp(vm->err_buf, 1))
    {
        vm_update_ctx(vm, &fp->base, bp);
        DISPATCH();
    }
    vm->ret_val = NULL;
    vm->fp = NULL;
    return 1;
    do_call_uni:
    {
        op_stack_t target = SFETCH();
        callee = STACK(rcallable_t *, target);
        rtype_t *cl_type = r_typeof(callee);

        if(!rtype_is_callable(cl_type))
            vm_type_error(vm, "call", "callee", r_type_callable, cl_type);
        framesize = cl_type->sig->argsz;
        if(sp + framesize >= vm->stack + vm->stacksz)
            vm_error(vm, "call", "stack overflow.");
        memset(sp, 0, framesize);
        ip = vm_call(vm, cl_type->sig, ip, fp, sp, bp);
        sp += framesize;
        goto call_common;
    }
    do_call_fast:
    {
        op_stack_t target = SFETCH();
        callee = STACK(rcallable_t *, target);
        framesize = r_typeof(callee)->sig->argsz;
        goto call_common;
    }
    call_common:
    {
        *(vm_act_rec_t *)sp = (vm_act_rec_t)
            {
                .sp = sp - framesize,
                .fp = fp,
                .ip = ip,
                .bp = bp
            };
        sp += sizeof(vm_act_rec_t);
        bp = sp;
        if(rcall_is_builtin(callee))
            goto call_builtin;
        goto call_vm;
    }
    call_vm:
    {
        rclosure_t *cl = (rclosure_t *)callee;
        op_offset_t loc_sz = cl->fn->loc_sz;

        fp = cl;
        ip = cl->fn->code;
        sp += loc_sz;
        if(sp >= vm->stack + vm->stacksz)
            vm_error(vm, "call", "stack overflow.");
        memset(sp - loc_sz, 0, loc_sz);
        vm_update_ctx(vm, &fp->base, bp);
        DISPATCH();
    }
    call_builtin:
    {
        rbuiltin_t *bi = (rbuiltin_t *)callee;

        if(sp >= vm->stack + vm->stacksz - sizeof(double))
            vm_error(vm, "call", "stack overflow.");
        pret = sp;
        vm_update_ctx(vm, callee, bp);
        bi->fn(vm, bi, bp - sizeof(vm_act_rec_t), pret);
        goto ret_common;
    }
    do_ret:
    {
        op_stack_t result = SFETCH();
        pret = bp + result;
        goto ret_common;
    }
    ret_common:
    {
        vm_act_rec_t *rec = ((vm_act_rec_t *)bp) - 1;
        sp = rec->sp;
        fp = rec->fp;
        ip = rec->ip;
        bp = rec->bp;
        vm_update_ctx(vm, &fp->base, bp);
        if(!fp)
        {
            vm->ret_val = *(robject_t **)pret;
            vm->fp = NULL;
            return 0;
        }
        DISPATCH();
    }
    do_complete_box:
    {
        op_stack_t target = SFETCH(), dest = SFETCH();
        rtype_t *cl_type = r_typeof(STACK(rcallable_t *, target));
        rtype_t *ret_type = cl_type->sig->ret_type;
        if(rtype_is_scalar(ret_type))
        {
            robject_t *box = r_box(ret_type, pret);
            STACK(robject_t *, dest) = box;
        }
        else
        {
            STACK(robject_t *, dest) = *(robject_t **)pret;
        }
        DISPATCH();
    }
    do_missing:
    {
        op_stack_t dest = SFETCH();
        op_stack_t src = SFETCH();
        op_offset_t idx = OFETCH();
        argbits_t bits = STACK(argbits_t, src);
        rboolean_t val = argbits_missing(bits, idx);
        STACK(rboolean_t, dest) = val;
        DISPATCH();
    }
    do_frame:
    {
        op_offset_t size = OFETCH();
        argbits_t bits = FETCH(argbits_t);
        if(sp + size >= vm->stack + vm->stacksz)
            vm_error(vm, "frame", "stack overflow.");
        memset(sp, 0, size);
        *((argbits_t *)sp) = bits;
        sp += size;
        DISPATCH();
    }
    do_jump:
    {
        op_offset_t addr = OFETCH();
        ip = fp->fn->code + addr;
        DISPATCH();
    }
    do_if:
    {
        op_stack_t pred = SFETCH();
        op_offset_t addr = OFETCH();
        rboolean_t val = STACK(rboolean_t, pred);
        if(val == true)
            ip = fp->fn->code + addr;
        else if(val == rboolean_na)
            vm_error(vm, "if", "NA value found where true/false expected.");
        DISPATCH();
    }
    do_lambda:
    {
        op_stack_t dest = SFETCH();
        op_offset_t idx = OFETCH();
        rfunction_t *fn = (rfunction_t *)fp->fn->consts[idx];
        rclosure_t *value = rcall_closure_create(fn->cl_type, fn);

        ip = populate_closure(value->env, ip, bp);
        STACK(rclosure_t *, dest) = value;
        DISPATCH();
    }
    do_defvar:
    {
        op_offset_t nidx = OFETCH(), tidx = OFETCH();
        rsymbol_t *sym = (rsymbol_t *)fp->fn->consts[nidx];
        rtype_t *type = (rtype_t *)fp->fn->consts[tidx];

        if(r_get_global(sym))
            vm_error(vm, "def", "can't define `%s`.", r_symstr(sym));
        r_create_global(sym, type, false);
        DISPATCH();
    }
    do_getvar_uni:
    {
        op_stack_t dest = SFETCH();
        op_offset_t idx = OFETCH();
        rsymbol_t *sym = (rsymbol_t *)fp->fn->consts[idx];
        rglobal_t *global = r_get_global(sym);

        if(!global)
            vm_error(vm, "get", "global `%s` not defined.", r_symstr(sym));
        if(global->decl && rtype_is_scalar(global->decl))
            STACK(robject_t *, dest) = r_box(global->decl, &global->val);
        else
            STACK(robject_t *, dest) = global->val.object;
        DISPATCH();
    }
    do_setvar_uni:
    {
        op_stack_t src = SFETCH();
        op_offset_t idx = OFETCH();
        rsymbol_t *sym = (rsymbol_t *)fp->fn->consts[idx];
        robject_t *obj = STACK(robject_t *, src);
        rglobal_t *global = r_get_global(sym);

        if(!global)
            global = r_create_global(sym, NULL, false);
        else if(global->is_const)
            vm_error(vm, "set", "invalid assignment to `const %s`.",
                     r_symstr(sym));
        if(global->decl && rtype_is_scalar(global->decl))
        {
            rtype_t *type = r_typeof(obj), *decl = global->decl;
            void *ptr = &global->val;

            if(decl == type)
                r_unbox(ptr, obj);
            else if(rtype_is_scalar(type))
                scalar_convert(ptr, obj, decl, type);
            else
                vm_type_error(vm, "unbox", "value", decl, type);
        }
        else
            global->val.object = obj;
        DISPATCH();
    }
    do_const:
    {
        op_stack_t dest = SFETCH();
        op_offset_t idx = OFETCH();

        STACK(robject_t *, dest) = fp->fn->consts[idx];
        DISPATCH();
    }
    do_cellmake:
    {
        op_stack_t dest = SFETCH();
        op_offset_t idx = OFETCH();
        rtype_t *type = (rtype_t *)fp->fn->consts[idx];
        STACK(robject_t *, dest) = rcell_create(type);
        DISPATCH();
    }
    do_check:
    {
        op_stack_t src = SFETCH();
        op_offset_t idx = OFETCH();
        rtype_t *type = (rtype_t *)fp->fn->consts[idx];
        rtype_t *vtyp = r_typeof(STACK(robject_t *, src));

        if(!r_subtypep(vtyp, type))
            vm_type_error(vm, "check", "value", type, vtyp);
        DISPATCH();
    }
    DO_BINOP(and, boolean, boolean)
    DO_BINOP(or, boolean, boolean)
    DO_UNOP(not, boolean, boolean)
    OPS_CONVERSION(DO_CONVOP)
    #define op_w 8
    #include "vm/op_data.c.inc"
    #undef op_w
    #define op_w 32
    #include "vm/op_data.c.inc"
    #undef op_w
    #define op_w 64
    #include "vm/op_data.c.inc"
    #undef op_w
    #define op_t ptr
    #include "vm/op_getelt.c.inc"
    #undef op_t
    #define op_t double
    #include "vm/op_scalar.c.inc"
    #include "vm/op_getelt.c.inc"
    #include "vm/op_arith.c.inc"
    #undef op_t
    #define op_t int
    #include "vm/op_scalar.c.inc"
    #include "vm/op_getelt.c.inc"
    #include "vm/op_arith.c.inc"
    #undef op_t
    #define op_t boolean
    #include "vm/op_scalar.c.inc"
    #include "vm/op_getelt.c.inc"
    #undef op_t
}
