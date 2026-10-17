#ifndef MAI_H
#define MAI_H
#define MAI_EXPORT __attribute__((visibility("default")))
enum
{
    MAI_OK = 0,     // success
    MAI_EPARSE = 1, // syntax or compile error
    MAI_ERUN = 2,   // run-time error
    MAI_EARG = 3    // bad argument, unknown or incompatible global
};
typedef enum
{
    MAI_NONE, MAI_BOOLEAN, MAI_INT, MAI_DOUBLE, MAI_OTHER
} mai_type_t;
typedef struct
{
    mai_type_t type;
    int length;         // number of elements
    int rank;           // 0 scalar, 1 vector, 2 or more array
    const int *shape;   // rank extents; NULL unless rank >= 2
    void *data;         // the elements
} mai_value_t;
MAI_EXPORT void mai_init(void);
MAI_EXPORT void mai_fini(void);
MAI_EXPORT int mai_eval(const char *code, char **out);
MAI_EXPORT int mai_get(const char *name, mai_value_t *val);
MAI_EXPORT int mai_set(const char *name, const mai_value_t *val);
MAI_EXPORT void mai_free(void *ptr);
#endif
