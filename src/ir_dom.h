typedef struct
{
    cblock_array_t *dfs;
    cblock_array_t *children;
} doms_t;
doms_t *cfunc_dom(cfunction_t *fn);
void dom_free(doms_t *dom, int nblocks);
