typedef struct
{
    unsigned start, end;
    list_t it_list;
} interval_t;
typedef struct
{
    cnode_t *node;
    list_t it_head;
    list_t range_list;
    unsigned start, end;
} range_t;
typedef struct
{
    range_t *ranges;
    list_t range_head;
} liveranges_t;
static inline range_t *range_for(liveranges_t *live, cnode_t *node)
    { return &live->ranges[node->id]; }
bool range_covers(range_t *range, unsigned point);
void liveranges_free(cfunction_t *fn, liveranges_t *live);
liveranges_t *cfunc_liveranges(cfunction_t *fn);
op_stack_t *gen_locations(cfunction_t *fn, liveranges_t *live,
                          op_offset_t *pscalsz, op_offset_t *psz); // HACK
