typedef struct __attribute__ ((__packed__))
{
    // optimisation
    bool opt_builtin;
    bool opt_inline;
    bool opt_dce;
    bool opt_constfold;
    bool opt_dvn;
    // debugging
    bool dbg_dump_ir;
    // printing
    int print_digits;
    int print_line;
    int print_max;
    int print_name_max;
} options_t;
extern options_t opt;
void rt_init_options();
