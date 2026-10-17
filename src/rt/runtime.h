typedef double rdouble_t;
typedef int rint_t;
typedef uint8_t rboolean_t;
typedef struct rtype rtype_t;
typedef struct
{
    rtype_t *type;
} robject_t;
typedef ARRAY(robject_t *) robject_array_t;
typedef robject_t *rptr_t;
#define rptr_na NULL
typedef struct
{
    uint32_t (*hash)(const void *);
    bool (*equal)(const void *, const void *);
    void (*print)(FILE *, const void *);
    void (*gc)(void *);
    void (*free)(void *);
} typeops_t;
typedef struct rsymbol
{
    robject_t base;
    char *string;
    uint32_t hash;
} rsymbol_t;
typedef struct
{
    robject_t base;
    char *string;
} rstring_t;
typedef union {
    rboolean_t boolean;
    rint_t integer;
    rdouble_t dfloat;
    robject_t *object;
} rvalue_union_t;
typedef struct
{
    rvalue_union_t val;
    rtype_t *decl;
    bool is_const;
} rglobal_t;
extern hashmap_t *r_globals;
extern hashset_t *r_symtab;
extern rsymbol_t *r_sym_rest;
extern rtype_t *r_type_object, *r_type_symbol, *r_type_string;
extern rtype_t *r_type_nil, *r_type_cell;
uint32_t rsym_hash(const void *sym);
rsymbol_t *r_intern(const char *string);
rstring_t *rstr_create(const char *string);
void r_free(void *ptr);
void r_gc(void *ptr);
uint32_t r_hash(const void *ptr);
bool r_equal(const void *px, const void *py);
void r_print(FILE *fp, const void *ptr);
rglobal_t *r_get_global(rsymbol_t *name);
rglobal_t *r_create_global(rsymbol_t *name, rtype_t *decl, bool is_const);
bool r_unset(rsymbol_t *name);
void r_definitial(rsymbol_t *name, rtype_t *decl, void *pval);
rtype_t *rcell_type_create(rtype_t *etyp);
robject_t *rcell_create(rtype_t *etyp);
void runtime_init();
void runtime_fini();
#define RTYPE(type) PASTE2(r_type_, type)
static inline uint32_t hash_roll(uint32_t hash, uint32_t val)
    { return hash_code_seed(&val, sizeof(val), hash); }
static inline char *r_symstr(const rsymbol_t *sym)
    { return sym ? sym->string : "(nil)"; }
static inline rtype_t *r_typeof(const void *ptr)
    { return ptr ? ((robject_t *)ptr)->type : r_type_nil; }
static inline void r_defbuiltin(char *str, void *value)
    { r_definitial(r_intern(str), r_typeof(value), &value); }
static inline void r_defscalar(char *str, rtype_t *decl,
                               rvalue_union_t value)
    { r_definitial(r_intern(str), decl, &value); }
#include "rt/type.h"
#include "rt/scalar.h"
#include "rt/callable.h"
#include "rt/call.h"
#include "rt/vector.h"
#include "rt/gc.h"
#include "rt/math.h"
#include "rt/options.h"
#include "vm/vm.h"
