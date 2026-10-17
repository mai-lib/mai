typedef struct gen_copy
{
    list_t list;
    op_stack_t dest, src;
    rtype_t *type;
} gen_copy_t;
typedef struct gen_fixup
{
    SLIST(struct gen_fixup) next;
    op_offset_t addr;
} gen_fixup_t;
typedef struct
{
    op_offset_t addr;
    bool valid;
    list_t copies;
    SLIST(gen_fixup_t) fixups;
} gen_block_t;
typedef struct
{
    op_code_array_t code;
    gen_block_t *records;
    op_stack_t *locs;
    op_offset_t loc_sz;
    op_offset_t *env_ofs;
    robject_array_t *consts;
    unsigned nfuncs;
    cvar_array_t *closure;
} gen_ctx_t;
extern gen_ctx_t ctx;
static inline gen_block_t *rec_for(cblock_t *block)
    { return &ctx.records[block->id]; }
static inline op_stack_t loc_for(cnode_t *node)
    { return ctx.locs[node->id]; }
static inline size_t asm_len()
    { return alen(&ctx.code); }
static inline uint8_t *asm_extend(size_t sz)
{
    size_t len = asm_len();
    return array_extend(&ctx.code, sz) + len;
}
static inline void asm_literal(void *ptr, size_t sz)
{
    uint8_t *dest = asm_extend(sz);
    memcpy(dest, ptr, sz);
}
static inline void asm_subop(op_code_t v)
{
    *(asm_extend(sizeof(op_code_t))) = v;
}
static inline void asm_op(op_code_t v)
{
    *(const void **)(asm_extend(sizeof(vm_instr_t))) = vm_instr[v];
}
static inline void asm_stack(op_stack_t v)
{
    *(op_stack_t *)(asm_extend(sizeof(v))) = v;
}
static inline void asm_offset(op_stack_t v)
{
    *(op_offset_t *)(asm_extend(sizeof(v))) = v;
}
static inline int width_code(rtype_t *type)
{
    switch(rtype_eltsz(type))
    {
    case 1: return 0;
    case 4: return 1;
    case 8: return 2;
    default: return -1;
    } /* NOTREACHED */
}
static inline void asm_op_width(op_code_t op, rtype_t *type)
    { asm_op(op + op_width_ofs[width_code(type)]); }
static inline void asm_op_type(op_code_t op, rtype_t *type)
    { assert(rtype_is_scalar(type)); asm_op(op + op_type_ofs[rscal_code(type)]); }
static inline void asm_op_conv(rtype_t *stype, rtype_t *dtype)
{
    assert(rtype_is_scalar(stype) && rtype_is_scalar(dtype));
    scalar_code scode = rscal_code(stype), dcode = rscal_code(dtype);
    scalar_code rcode = scode * (SC_MAX-1) + dcode - (scode < dcode);
    asm_op(op_conv_ofs + rcode);
}
static inline void asm_args(cnode_array_t *args)
{
    cnode_t *arg;
    array_foreach_entry(args, arg)
        asm_stack(loc_for(arg));
}
static inline void asm_patch(op_offset_t addr, op_offset_t dest)
    { *(op_offset_t *)(ctx.code.ptr + addr) = dest; }
op_offset_t const_index(void *val);
void asm_dump(cfunction_t *fn, op_code_array_t *code, robject_array_t *consts); // DEBUG
