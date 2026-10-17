typedef struct vm_ctx
{
    uint8_t *stack;
    size_t stacksz;
    robject_t *ret_val;
    char *err_msg;
    sigjmp_buf err_buf;
    gc_root_t gc_root;
    robject_array_t retained;
    rcallable_t *fp;
    uint8_t *bp;
} vm_ctx_t;
void vm_init_ctx(vm_ctx_t *ctx, uint8_t *stack, size_t stacksz);
void vm_fini_ctx(vm_ctx_t *ctx);
void *vm_retain(vm_ctx_t *ctx, void *obj);
void vm_release(vm_ctx_t *ctx, int n);
int vm_execute(vm_ctx_t *vm, rclosure_t *closure);
void vm_error(vm_ctx_t *vm, const char *src, const char *fmt, ...);
