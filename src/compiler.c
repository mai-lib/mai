#include "global.h"
#include "ir.h"
#include "ast.h"
static const char *lvlnames[] = { "Error", "Warning" };
static SLIST(cmessage_t) c_msgs;
static hashset_t *c_consttab;
void *c_intern(void *ptr)
{
    void *obj = hashset_insert(c_consttab, ptr);
    if(obj != ptr)
        gc_release(ptr);
    return obj;
}
FILE *c_msgfp;
void c_messages(int level)
{
    if(c_msgs)
    {
        cmessage_t *msg, *tmp;

        slist_nreverse(c_msgs, next);
        slist_foreach_safe(c_msgs, msg, tmp, next)
        {
            if(msg->level <= level)
                fprintf(c_msgfp ? c_msgfp : stderr, "%s: %s\n", lvlnames[msg->level], msg->string);
            xfree(msg->string);
            xfree(msg);
        }
        c_msgs = NULL;
    }
}
void c_message_va(cmsglevel lvl, rsymbol_t *file, char *fmt, ...)
{
    va_list va;
    cmessage_t *msg = xcalloc(1, sizeof(*msg));

    va_start(va, fmt);
    msg->level = lvl;
    msg->file = file;
    if(vasprintf(&msg->string, fmt, va) == -1)
        fatal("vasprintf failed.");
    slist_push(c_msgs, msg, next);
    va_end(va);
}
void c_begin()
{
    c_msgs = NULL;
    c_consttab = hashset_create(r_hash, r_equal);
}
void c_end()
{
    hashset_free(c_consttab);
    c_messages(C_WARNING);
    assert(c_msgs == NULL);
}
cfunction_t *ir_convert(ast_t *ast, char *filename); // ir_convert.c
cresult ir_prepare(cfunction_t *fn); // ir_pre.c
cresult ir_postpass(cfunction_t *fn); // opt_post.c
void ir_init_closure(cfunction_t *fn); // ir_closure.c
void ir_ssa_convert(cfunction_t *fn); // ir_ssa.c
cresult ir_optimise(cfunction_t *fn); // opt_sccp.c
cresult ir_cell_intro(cfunction_t *fn); // opt_closure.c
void ir_dvn(cfunction_t *fn); // opt_dvn.c
rfunction_t *gen_function(cfunction_t *fn); // gen_code.c
rclosure_t *compile(ast_t *ast, char *name)
{
    cfunction_t *fn = NULL;
    rfunction_t *rfn = NULL;
    rclosure_t *cl = NULL;

    fn = ir_convert(ast, name);
    ast_fini(ast);
    if(!fn)
        goto fail;
    if(failed(ir_prepare(fn)))
        goto fail;
    ir_ssa_convert(fn);
    if(failed(ir_optimise(fn)))
        goto fail;
    ir_cell_intro(fn);
    if(failed(ir_postpass(fn)))
        goto fail;
    ir_dvn(fn);
    if(opt.dbg_dump_ir)
        ir_dump(fn);
    rfn = gen_function(fn);
    cl = rcall_closure_create(rfn->cl_type, rfn);
fail:
    if(fn)
        cfunc_free(fn);
    return cl;
}
