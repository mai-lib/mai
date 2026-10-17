typedef struct
{
    argbits_t posbits; // arguments unavailable for use by positionals
    argbits_t argbits; // arguments present in call
    funsig_t *sig; // signature of the function being called
    bool (*append_rest)(void *, rsymbol_t *, void *);
    bool (*set_arg)(void *, argdesc_t *, void *);
} call_ctx_t;

// first arg is a call_ctx_t *
// the void * decl is so you can use them as callbacks with call_sequence
bool call_match_kwd(void *ptr, rsymbol_t *name, void *val);
bool call_match_pos(void *ptr, void *val);
bool call_match_omit(void *ptr);
bool call_match_rest(void *ptr, void **args, rsymbol_t **names, int nargs);
bool no_match_rest(void *ptr, void *rest);

bool call_sequence(void *ptr, void **args, rsymbol_t **names, int nargs,
                   bool (*rest_fn)(void *, void *),
                   bool (*kwd_fn)(void *, rsymbol_t *, void *),
                   bool (*pos_fn)(void *, void *),
                   bool (*omit_fn)(void *));
