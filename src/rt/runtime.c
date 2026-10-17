#include "global.h"
#include "builtin.h"
uint32_t r_hash(const void *ptr)
{
    rtype_t *typ = r_typeof(ptr);
    assert(typ->ops->hash);
    return typ->ops->hash(ptr);
}
bool r_equal(const void *xp, const void *yp)
{
    const robject_t *x = xp, *y = yp;
    rtype_t *xt = r_typeof(x), *yt = r_typeof(y);

    if(x == y)
        return true;
    if(xt != yt)
        return false;
    if(xt->ops->equal)
        return xt->ops->equal(xp, yp);
    return false;
}
void r_print(FILE *fp, const void *ptr)
{
    rtype_t *typ = r_typeof(ptr);

    if(typ->ops->print)
        typ->ops->print(fp, ptr);
    else
    {
        fprintf(fp, "<");
        r_print(fp, typ);
        fprintf(fp, " %p>", ptr);
    }
}
void r_gc(void *ptr)
{
    rtype_t *typ = r_typeof(ptr);

    gc_mark(typ);
    if(typ->ops->gc)
        typ->ops->gc(ptr);
}
void r_free(void *ptr)
{
    rtype_t *typ = r_typeof(ptr);
    if(typ->ops->free)
        typ->ops->free(ptr);
}
static void mark_global(const void *key, void *value, void *data)
{
    rglobal_t *global = value;

    gc_mark((void *)key);
    gc_mark(global->decl);
    if(!global->decl || !rtype_is_scalar(global->decl))
        gc_mark(global->val.object);
}
static void mark_type(const void *key, void *value, void *data)
{
    gc_mark((void *)key);
    gc_mark(value);
}
static void runtime_gc(gc_root_t *root)
{
    hashmap_map(r_globals, mark_global, NULL);
    hashmap_map(r_global_types, mark_type, NULL);
    gc_mark(r_type_cell);
    gc_mark(r_type_function);
    gc_mark(r_sym_rest);
}
static gc_root_t runtime_root = { .fn = runtime_gc };
static uint32_t nil_hash(const void *ptr)
{
    return 0;
}

static void nil_print(FILE *fp, const void *ptr)
{
    fprintf(fp, "<nil>");
}

static const typeops_t nil_ops = {
    .hash = nil_hash,
    .print = nil_print
};
hashset_t *r_symtab;
static void sym_init(rsymbol_t *sym, char *string, uint32_t hash)
{
    sym->string = string;
    sym->hash = hash;
}
static bool sym_equal(const void *xp, const void *yp)
{
    const rsymbol_t *x = xp, *y = yp;
    return string_equal(x->string, y->string);
}
uint32_t rsym_hash(const void *ptr)
{
    return ((rsymbol_t *)ptr)->hash;
}
rsymbol_t *r_intern(const char *string)
{
    assert(string);
    rsymbol_t *sym, tmp;
    uint32_t hash = string_hash(string);

    sym_init(&tmp, (char *)string, hash);
    sym = hashset_get(r_symtab, &tmp);
    if(sym)
        return sym;
    sym = gc_alloc(r_type_symbol, sizeof(rsymbol_t));
    sym_init(sym, strdup(string), hash);
    hashset_insert(r_symtab, sym);
    return sym;
}
static void sym_free(void *ptr)
{
    rsymbol_t *sym = ptr;

    hashset_remove(r_symtab, sym);
    xfree(sym->string);
}
static void sym_print(FILE *fp, const void *ptr)
{
    fprintf(fp, "%s", r_symstr(ptr));
}
static const typeops_t sym_ops = {
    .free = sym_free,
    .hash = rsym_hash,
    .equal = NULL,
    .print = sym_print
};
rstring_t *rstr_create(const char *string)
{
    rstring_t *str = gc_alloc(r_type_string, sizeof(rstring_t));
    str->string = strdup(string);
    return str;
}
static void rstr_free(void *ptr)
{
    rstring_t *str = ptr;
    xfree(str->string);
}
static bool rstr_equal(const void *xp, const void *yp)
{
    const rstring_t *x = xp, *y = yp;
    return string_equal(x->string, y->string);
}
static uint32_t rstr_hash(const void *ptr)
{
    const rstring_t *str = ptr;
    return string_hash(str->string);
}
static void rstr_print(FILE *fp, const void *ptr)
{
    const rstring_t *str = ptr;
    fprintf(fp, "\"%s\"", str->string);
}
static const typeops_t str_ops = {
    .free = rstr_free,
    .hash = rstr_hash,
    .equal = rstr_equal,
    .print = rstr_print
};
robject_t *rcell_create(rtype_t *typ)
{
    assert(rtype_is_cell(typ));
    return gc_alloc(typ, sizeof(robject_t) + rtype_eltsz(typ->elt));
}
static void rcell_gc(void *ptr)
{
    robject_t *cell = ptr;
    rtype_t *etyp = r_typeof(cell)->elt;

    if(!rtype_is_scalar(etyp))
        gc_mark(UNBOX(robject_t *, cell));
}
static const typeops_t cell_ops = {
    .gc = rcell_gc,
};
static consdesc_t cell_cons = {
    .kind = RT_CELL
};
rtype_t *rcell_type_create(rtype_t *etyp)
{
    return rtype_create(RT_CELL, etyp, &cell_ops, NULL);
}
hashmap_t *r_globals;
rglobal_t *r_get_global(rsymbol_t *name)
{
    return hashmap_get(r_globals, name, NULL);
}
rglobal_t *r_create_global(rsymbol_t *name, rtype_t *decl, bool is_const)
{
    rglobal_t *global = xcalloc(1, sizeof(rglobal_t));

    assert(!r_get_global(name));
    *global = (rglobal_t) {
        .decl = decl,
        .is_const = is_const
    };
    hashmap_set(r_globals, name, global);
    return global;
}
bool r_unset(rsymbol_t *name)
{
    rglobal_t *global = r_get_global(name);

    if(global && !global->decl)
    {
        hashmap_remove(r_globals, name);
        xfree(global);
        return true;
    }
    return false;
}
void r_definitial(rsymbol_t *name, rtype_t *decl, void *pval)
{
    assert(decl);
    rglobal_t *global = r_create_global(name, decl, true);
    memcpy(&global->val, pval, rtype_eltsz(decl));
}
rtype_t *r_type_nil, *r_type_symbol, *r_type_string;
rtype_t *r_type_cell, *r_type_object;
rsymbol_t *r_sym_rest;
const extern builtin_init_t runtime_builtins[];
const extern builtin_init_t arith_builtins[];
void runtime_init()
{
    r_globals = hashmap_create(rsym_hash, ptr_eq);
    r_symtab = hashset_create(rsym_hash, sym_equal);
    gc_init();
    gc_register(&runtime_root);
    vm_execute(NULL, NULL);
    rt_bootstrap();
    r_type_symbol = rtype_create(RT_OBJECT, NULL, &sym_ops, "symbol");
    rtype_install(r_type_type, "type");
    rtype_install(r_type_symbol, "symbol");
    r_type_object = rtype_init(RT_OBJECT, NULL, "object");
    r_type_nil = rtype_init(RT_OBJECT, &nil_ops, "nil");
    r_type_string = rtype_init(RT_OBJECT, &str_ops, "string");
    r_type_cell = rtype_cons_create(&cell_cons, "cell");
    r_defbuiltin("nil", NULL);
    r_sym_rest = r_intern("...");
    rt_install_call_types();
    rt_install_scalar_types();
    rt_install_vec_types();
    rbuiltin_install(arith_builtins);
    rbuiltin_install(runtime_builtins);
    rt_init_options();
}
static void free_global(const void *key, void *value, void *data)
{
    xfree(value);
}

void runtime_fini()
{
    gc_fini();
    hashmap_map(r_globals, free_global, NULL);
    hashmap_free(r_globals);
    hashset_free(r_symtab);
    hashset_free(r_typetab);
    hashmap_free(r_global_types);
}
