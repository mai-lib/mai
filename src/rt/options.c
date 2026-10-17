#include "global.h"
#include <ctype.h>
#include "rt/builtin.h"
#include "arith/scalar.h"
#include "arith/vec.h"

options_t opt;

static inline void *opt_ofs(argdesc_t *arg)
{
    return (uint8_t *)&opt + arg->offset - sizeof(argbits_t);
}

// return a named vector containing the compiler options
static rvector_t *get_options(vm_ctx_t *vm, funsig_t *sig)
{
    int nargs = sig->nargs, i;
    rvector_t *res = vm_retain(vm, rvec_create(r_type_vec_object, nargs));
    robject_t **elts = rvec_elts(res);

    for(i=0; i<nargs; i++)
    {
        argdesc_t *arg = &sig->args[i];
        void *src = opt_ofs(arg);

        if(rtype_is_scalar(arg->type))
            elts[i] = vm_retain(vm, r_box(arg->type, src));
        else
            elts[i] = *(robject_t **)src;
    }
    // ensure the vector is fully initialised before calling rvec_add_names...
    rsymbol_t **names = rvec_elts(rvec_add_names(res));
    for(i=0; i<nargs; i++)
        names[i] = sig->args[i].name;
    vm_release(vm, 1+nargs);
    return res;
}

// set the given compiler options, returning the previous state
// so it can be reset with something like `apply(options, state)`
static void builtin_options(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    funsig_t *sig = rcall_sig(&fn->base);
    robject_t *res = vm_retain(vm, get_options(vm, sig));
    args -= sig->argsz; // XXX builtin call convention is silly, fix this imo
    argbits_t argbits = *(argbits_t *)args;
    assert(sizeof(opt) == sig->argsz - sizeof(argbits_t));

    for(int i=0; i<sig->nargs; i++)
    {
        if(argbits & (1<<i))
        {
            argdesc_t *arg = &sig->args[i];
            void *src = args + arg->offset;
            void *dest = opt_ofs(arg);

            if(rtype_is_scalar(arg->type)
               && scalar_is_na(arg->type, src) == true)
            {   // XXX vm_warning?
                fprintf(stderr, "warning: argument `%s` not valid.\n",
                        r_symstr(arg->name));
                continue;
            }
            memcpy(dest, src, rtype_eltsz(arg->type));
        }
    }
    vm_release(vm, 1);
    builtin_return(r, robject_t *, res);
}

// XXX multiexpand to avoid these getting out of sync
// (typedef in rt/options.h also)
const builtin_init_t options_builtin =
    defbuiltin("options", builtin_options, &r_type_vec_object,
               { "opt_builtin", &r_type_boolean, true },
               { "opt_inline", &r_type_boolean, true },
               { "opt_dce", &r_type_boolean, true },
               { "opt_constfold", &r_type_boolean, true },
               { "opt_dvn", &r_type_boolean, true },
               { "dbg_dump_ir", &r_type_boolean, true },
               { "print_digits", &r_type_int, true },
               { "print_line", &r_type_int, true },
               { "print_max", &r_type_int, true },
               { "print_name_max", &r_type_int, true });

/*
static char *upcase(char *str)
{
    static char buf[32];
    int sz = sizeof(buf)-1;
    char *ptr = buf;

    while(*str != '\0' && sz-- > 0)
        *ptr++ = toupper(*str++);
    *ptr = '\0'; // terminate
    return buf;
}
*/

static int init_option(char *name, int def)
{
    char *val = getenv(name);
    if(val)
        return atoi(val);
    return def;
}

void rt_init_options()
{
    opt = (options_t) {
        .opt_builtin = init_option("OPT_BUILTIN", true),
        .opt_inline = init_option("OPT_INLINE", true),
        .opt_dce = init_option("OPT_DCE", true),
        .opt_constfold = init_option("OPT_CONSTFOLD", true),
        .opt_dvn = init_option("OPT_DVN", true),
        .dbg_dump_ir = init_option("DBG_DUMP_IR", false),
        .print_digits = 6,
        .print_line = 76,
        .print_max = 200,
        .print_name_max = 15,
    };

    rbuiltin_define(&options_builtin);
/*
    rbuiltin_t *fn = (rbuiltin_t *)rbuiltin_define(&options_builtin);
    funsig_t *sig = fn->base.base.type->sig;
    for(int i=0; i<sig->nargs; i++)
    {
        argdesc_t *arg = &sig->args[i];
        char *str = getenv(upcase(r_symstr(arg->name)));
        if(str)
        {
            void *dest = opt_ofs(arg);
            // ... atoi ...
        }
    }
*/
}
