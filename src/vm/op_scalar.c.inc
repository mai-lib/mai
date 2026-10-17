DO_BINOP(eql, op_t, boolean)
DO_BINOP(neq, op_t, boolean)
DO_UNOP(is_na, op_t, boolean)
LABEL(box, op_t):
{
    op_stack_t dest = SFETCH(), src = SFETCH();
    robject_t *obj = r_box_create(RTYPE(op_t));
    UNBOX(op_type_t, obj) = STACK(op_type_t, src);
    STACK(robject_t *, dest) = obj;
    DISPATCH();
}
LABEL(unbox, op_t):
{
    op_stack_t dest = SFETCH(), src = SFETCH();
    robject_t *obj = STACK(robject_t *, src);
    rtype_t *type = r_typeof(obj), *decl = RTYPE(op_t);
    op_type_t *ptr = &STACK(op_type_t, dest);
    if(decl == type)
        *ptr = UNBOX(op_type_t, obj);
    else if(rtype_is_scalar(type))
        scalar_convert(ptr, obj, decl, type);
    else
        vm_type_error(vm, "unbox", "value", decl, type);
    DISPATCH();
}
