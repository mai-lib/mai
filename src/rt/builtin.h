typedef struct
{
    char *name;
    rtype_t **ptype;
    bool is_optional;
} builtin_init_arg_t;
typedef struct
{
    char *name;
    builtin_fn fn;
    rtype_t **prtype;
    const cbuiltin_t *cbi;
    const builtin_init_arg_t *args;
} builtin_init_t;
#define init_builtin(n, f, prt, a...) \
    .name = n, .fn = f, .prtype = prt, .args = a
#define init_args(a...) (builtin_init_arg_t []) { a, { } }
#define init_cbi(o, n, d) \
    &(cbuiltin_t) { .ops = o, .name = n, .data = d }
#define defbuiltin(n, f, prt, a...) \
    { init_builtin(n, f, prt, init_args(a)) }
#define defcbuiltin(n, f, prt, o, d, a...) \
    { init_builtin(n, f, prt, init_args(a)), .cbi = init_cbi(o, n, d) }
#define def_args struct __attribute__ ((__packed__))
#define end_args(args, n) *n = (void *)(args - sizeof(*n))
#define builtin_return(ret, typ, val) (*((typ *)(ret))) = val
rcallable_t *rbuiltin_define(const builtin_init_t *init);
void rbuiltin_install(const builtin_init_t *init);
#define missingp(a,b) argbits_missing(a,b)
