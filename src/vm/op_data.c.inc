LABEL(mov, op_w):
{
    op_stack_t dest = SFETCH(), src = SFETCH();
    STACK(op_width_t, dest) = STACK(op_width_t, src);
    DISPATCH();
}
LABEL(literal, op_w):
{
    op_stack_t dest = SFETCH();
    op_width_t value = FETCH(op_width_t);
    STACK(op_width_t, dest) = value;
    DISPATCH();
}
LABEL(env, op_w):
{
    op_stack_t dest = SFETCH();
    op_offset_t ofs = OFETCH();
    STACK(op_width_t, dest) = *(op_width_t *)(((uint8_t *)fp->env) + ofs);
    DISPATCH();
}
LABEL(complete, op_w):
{
    op_stack_t dest = SFETCH();
    STACK(op_width_t, dest) = *(op_width_t *)pret;
    DISPATCH();
}
LABEL(getvar, op_w):
{
    op_stack_t dest = SFETCH();
    op_offset_t idx = OFETCH();
    rsymbol_t *sym = (rsymbol_t *)fp->fn->consts[idx];
    rglobal_t *global = r_get_global(sym);

    if(!global)
        vm_error(vm, "get", "global `%s` not defined.",
                 r_symstr(sym));
    STACK(op_width_t, dest) = *(op_width_t *)(&global->val);
    DISPATCH();
}
LABEL(setvar, op_w):
{
    op_stack_t src = SFETCH();
    op_offset_t idx = OFETCH(), flag = OFETCH();
    rsymbol_t *sym = (rsymbol_t *)fp->fn->consts[idx];
    rglobal_t *global = r_get_global(sym);

    if(!global)
        vm_error(vm, "set", "global `%s` not defined.",
                 r_symstr(sym));
    else if(global->is_const)
        vm_error(vm, "set", "invalid assignment to `const %s`.",
                 r_symstr(sym));
    *(op_width_t *)(&global->val) = STACK(op_width_t, src);
    if(flag)
        global->is_const = true;
    DISPATCH();
}
LABEL(cellget, op_w):
{
    op_stack_t dest = SFETCH(), src = SFETCH();
    robject_t *cell = STACK(robject_t *, src);

    STACK(op_width_t, dest) = UNBOX(op_width_t, cell);
    DISPATCH();
}
LABEL(cellset, op_w):
{
    op_stack_t dest = SFETCH(), src = SFETCH();
    robject_t *cell = STACK(robject_t *, dest);

    UNBOX(op_width_t, cell) = STACK(op_width_t, src);
    DISPATCH();
}
LABEL(setelt, op_w):
{
    op_stack_t obj = SFETCH(), src = SFETCH(), idx = SFETCH();
    rbuf_t *buf = STACK(rbuf_t *, obj);
    if(!buf)
        vm_error(vm, "[=", "can't subscript nil.");
    int len = buf->length;
    int i = STACK(rint_t, idx);
    if(!int_na(i) && i > 0 && i <= len)
        ((op_width_t *)buf->elts)[i - 1] = STACK(op_width_t, src);
    DISPATCH();
}
LABEL(setel2, op_w):
{
    op_stack_t obj = SFETCH(), src = SFETCH(),
               row = SFETCH(), col = SFETCH();
    rarray_t *arr = STACK(rarray_t *, obj);
    int i = STACK(rint_t, row), j = STACK(rint_t, col);
    if(!arr)
        vm_error(vm, "[=", "can't subscript nil.");
    if(arr->rank != 2)
        vm_error(vm, "[=", "array not of rank 2.");
    if(!int_na(i) && i <= arr->shape[0] && i > 0 &&
       !int_na(j) && j <= arr->shape[1] && j > 0)
    {
        unsigned long idx = (unsigned long)arr->shape[0] * (j-1) + (i-1);
        ((op_width_t *)rvec_elts(arr))[idx] = STACK(op_width_t, src);
    }
    DISPATCH();
}
