LABEL(getelt, op_t):
{
    op_stack_t dest = SFETCH(), obj = SFETCH(), idx = SFETCH();
    rbuf_t *buf = STACK(rbuf_t *, obj);
    if(!buf)
        vm_error(vm, "[", "can't subscript nil.");
    int len = buf->length;
    int i = STACK(rint_t, idx);
    if(!int_na(i) && i <= len && i > 0)
        STACK(op_type_t, dest) = ((op_type_t *)buf->elts)[i - 1];
    else
        STACK(op_type_t, dest) = R_NA(op_t);
    DISPATCH();
}
LABEL(getel2, op_t):
{
    op_stack_t dest = SFETCH(), obj = SFETCH(),
               row = SFETCH(), col = SFETCH();
    rarray_t *arr = STACK(rarray_t *, obj);
    int i = STACK(rint_t, row), j = STACK(rint_t, col);
    if(!arr)
        vm_error(vm, "[", "can't subscript nil.");
    if(arr->rank != 2)
        vm_error(vm, "[", "array not of rank 2.");
    if(!int_na(i) && i <= arr->shape[0] && i > 0 &&
       !int_na(j) && j <= arr->shape[1] && j > 0)
    {
        unsigned long idx = (unsigned long)arr->shape[0] * (j-1) + (i-1);
        STACK(op_type_t, dest) = ((op_type_t *)rvec_elts(arr))[idx];
    }
    else
        STACK(op_type_t, dest) = R_NA(op_t);
    DISPATCH();
}
