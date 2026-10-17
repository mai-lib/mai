typedef enum { C_ERROR, C_WARNING } cmsglevel;
typedef struct cmessage
{
    SLIST(struct cmessage) next;
    cmsglevel level;
    rsymbol_t *file;
    char *string;
} cmessage_t;
typedef unsigned char cresult;
enum { SUCCESS = 0, FAILED = 1<<0, CHANGED = 1<<1 };
static inline bool failed(cresult res) { return (res & FAILED) != 0; }
static inline bool changed(cresult res) { return (res & CHANGED) != 0; }
#define c_warning(fmt, args...) c_message_va(C_WARNING, NULL, fmt, ##args)
#define c_error(fmt, args...) c_message_va(C_ERROR, NULL, fmt, ##args)
typedef struct cfunction cfunction_t;
cfunction_t *convert(ast_t *ast, char *name);
rclosure_t *source(char *name);
rclosure_t *compile(ast_t *ast, char *name);
void *c_intern(void *ptr);
void c_begin();
void c_end();
void c_messages(int level);
void c_message_va(cmsglevel lvl, rsymbol_t *file, char *fmt, ...);
