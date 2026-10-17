#include "global.h"
#include "ir.h"
#include "ast.h"
#include "mai.h"
#include <errno.h>
static uint8_t embed_stack[65536];
static bool embed_ready;
rclosure_t *source(char *filename)
{
    int r;
    rclosure_t *cl = NULL;
    ast_t ast;

    c_begin();
    r = p_source(filename, &ast);
    if(r == 0)
        cl = compile(&ast, filename);
    else if(r < 0)
        c_error("sourcing %s - %s", filename, strerror(errno));
    c_end();
    return cl;
}
void mai_init(void)
{
    if(embed_ready)
        return;
    runtime_init();
    embed_ready = true;
}
void mai_fini(void)
{
    if(!embed_ready)
        return;
    runtime_fini();
    embed_ready = false;
}
int mai_eval(const char *code, char **out)
{
    char *buf = NULL;
    size_t len = 0;
    FILE *os = open_memstream(&buf, &len);
    rclosure_t *cl = NULL;
    vm_ctx_t ctx;
    ast_t ast;
    int status;

    if(!code || !os)
    {
        if(os)
            fclose(os);
        free(buf);
        return MAI_EARG;
    }
    mai_init();
    c_msgfp = os;
    c_begin();
    if(p_string(code, &ast) == 0)
        cl = compile(&ast, "<embed>");
    c_end();
    c_msgfp = NULL;
    if(!cl)
        status = MAI_EPARSE;
    else
    {
        vm_init_ctx(&ctx, embed_stack, sizeof(embed_stack));
        gc_set_enabled(true);
        status = vm_execute(&ctx, cl) == 0 ? MAI_OK : MAI_ERUN;
        gc_set_enabled(false);
        vm_fini_ctx(&ctx);
        if(ctx.err_msg)
        {
            fputs(ctx.err_msg, os);
            xfree(ctx.err_msg);
            status = MAI_ERUN;
        }
        else if(status == MAI_OK)
            r_print(os, ctx.ret_val);
    }
    fclose(os);
    if(out)
        *out = buf;
    else
        free(buf);
    return status;
}
static mai_type_t mai_type_of(rtype_t *typ)
{
    if(typ == r_type_boolean)
        return MAI_BOOLEAN;
    if(typ == r_type_int)
        return MAI_INT;
    if(typ == r_type_double)
        return MAI_DOUBLE;
    return MAI_OTHER;
}
static rtype_t *mai_rtype(mai_type_t type)
{
    switch(type)
    {
    case MAI_BOOLEAN: return r_type_boolean;
    case MAI_INT: return r_type_int;
    case MAI_DOUBLE: return r_type_double;
    default: return NULL;
    }
}
int mai_get(const char *name, mai_value_t *val)
{
    rglobal_t *g;
    robject_t *obj;
    rtype_t *typ;

    if(!name || !val || !embed_ready)
        return MAI_EARG;
    g = r_get_global(r_intern(name));
    if(!g)
        return MAI_EARG;
    *val = (mai_value_t) { .type = MAI_OTHER };
    if(g->decl && rtype_is_scalar(g->decl))
    {
        *val = (mai_value_t) { mai_type_of(g->decl), 1, 0, NULL, &g->val };
        return MAI_OK;
    }
    obj = g->val.object;
    typ = r_typeof(obj);
    if(rtype_is_scalar(typ))
        *val = (mai_value_t) { mai_type_of(typ), 1, 0, NULL, BOXPTR(obj) };
    else if(rtype_is_vector(typ))
    {
        rvector_t *vec = (rvector_t *)obj;
        *val = (mai_value_t) { mai_type_of(typ->elt), rvec_len(vec), 1,
                               NULL, rvec_elts(vec) };
    }
    else if(rtype_is_array(typ))
    {
        rarray_t *arr = (rarray_t *)obj;
        *val = (mai_value_t) { mai_type_of(typ->elt), arr->buf.length,
                               arr->rank, arr->shape, arr->buf.elts };
    }
    if(val->type == MAI_OTHER)
        val->data = NULL;
    return MAI_OK;
}
int mai_set(const char *name, const mai_value_t *val)
{
    rtype_t *etyp, *typ;
    robject_t *obj;
    rglobal_t *g;
    rsymbol_t *sym;

    if(!name || !val || !embed_ready || !(etyp = mai_rtype(val->type)))
        return MAI_EARG;
    if(val->rank < 0 || val->length < 0 || (val->length && !val->data))
        return MAI_EARG;
    if(val->rank >= 2)
    {
        long n = 1;
        if(!val->shape)
            return MAI_EARG;
        for(int i = 0; i < val->rank; i++)
            n *= val->shape[i];
        if(n != val->length)
            return MAI_EARG;
    }
    else if(val->rank == 0 && val->length != 1)
        return MAI_EARG;
    sym = r_intern(name);
    g = r_get_global(sym);
    if(g && g->is_const)
        return MAI_EARG;
    if(g && g->decl && rtype_is_scalar(g->decl))
    {
        if(val->rank != 0 || g->decl != etyp)
            return MAI_EARG;
        memcpy(&g->val, val->data, rtype_eltsz(etyp));
        return MAI_OK;
    }
    if(val->rank == 0)
    {
        obj = r_box(etyp, val->data);
        typ = etyp;
    }
    else if(val->rank == 1)
    {
        typ = rvec_type_create(etyp);
        rvector_t *vec = rvec_create(typ, val->length);
        memcpy(rvec_elts(vec), val->data, val->length * rtype_eltsz(etyp));
        obj = (robject_t *)vec;
    }
    else
    {
        typ = rarr_type_create(etyp);
        rarray_t *arr = rarr_create(typ, val->rank, (int *)val->shape);
        memcpy(arr->buf.elts, val->data, val->length * rtype_eltsz(etyp));
        obj = (robject_t *)arr;
    }
    if(g && g->decl && g->decl != r_type_object && g->decl != typ)
        return MAI_EARG;
    if(!g)
        g = r_create_global(sym, r_type_object, false);
    g->val.object = obj;
    return MAI_OK;
}
void mai_free(void *ptr)
{
    free(ptr);
}
