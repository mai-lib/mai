/* Exercise the embedding interface in mai.h. Prints one line per check
   and exits non-zero if any fails. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mai.h"

static int fails;

static void check(int ok, const char *what)
{
    printf("%s  %s\n", ok ? "ok  " : "FAIL", what);
    if(!ok)
        fails++;
}

/* evaluate code, and check its status and (if want is not NULL) output */
static void eval_is(const char *code, int status, const char *want)
{
    char *out = NULL;
    int st = mai_eval(code, &out);
    int ok = st == status && (!want || (out && !strcmp(out, want)));

    if(!ok)
        printf("      status %d, output \"%s\"\n", st, out ? out : "(null)");
    check(ok, code);
    mai_free(out);
}

int main(void)
{
    mai_value_t v;

    mai_init();
    mai_init();
    eval_is("1 + 1", MAI_OK, "2");
    eval_is("const sq = function(x) x * x", MAI_OK, NULL);
    eval_is("sq(12)", MAI_OK, "144");
    eval_is("1 +", MAI_EPARSE, NULL);
    eval_is("no_such_thing + 1", MAI_ERUN,
            "get: global `no_such_thing` not defined.");

    double xs[] = { 1.5, 2.5, 3.0 };
    v = (mai_value_t) { MAI_DOUBLE, 3, 1, NULL, xs };
    check(mai_set("xs", &v) == MAI_OK, "set a double vector");
    eval_is("sum(xs)", MAI_OK, "7.0");
    eval_is("ys = xs * 2", MAI_OK, NULL);
    check(mai_get("ys", &v) == MAI_OK && v.type == MAI_DOUBLE &&
          v.rank == 1 && v.length == 3 && ((double *)v.data)[2] == 6.0,
          "get a double vector");

    int is[] = { 1, 2, 3, 4, 5, 6 }, shape[] = { 2, 3 };
    v = (mai_value_t) { MAI_INT, 6, 2, shape, is };
    check(mai_set("m", &v) == MAI_OK, "set an int array");
    eval_is("m[2, 3]", MAI_OK, "6");
    check(mai_get("m", &v) == MAI_OK && v.type == MAI_INT && v.rank == 2 &&
          v.shape[0] == 2 && v.shape[1] == 3, "get an int array");

    int n = 7;
    v = (mai_value_t) { MAI_INT, 1, 0, NULL, &n };
    check(mai_set("n", &v) == MAI_OK, "set an int scalar");
    eval_is("n + 1", MAI_OK, "8");
    check(mai_get("n", &v) == MAI_OK && v.type == MAI_INT && v.rank == 0 &&
          *(int *)v.data == 7, "get an int scalar");

    eval_is("const k = 2.5", MAI_OK, NULL);
    check(mai_get("k", &v) == MAI_OK && v.type == MAI_DOUBLE &&
          *(double *)v.data == 2.5, "get a const scalar");
    check(mai_set("k", &v) == MAI_EARG, "refuse to set a const");
    check(mai_get("sq", &v) == MAI_OK && v.type == MAI_OTHER && !v.data,
          "get a closure as other");
    check(mai_get("undefined", &v) == MAI_EARG, "get an undefined global");

    eval_is("var big = dbl(100000)", MAI_OK, NULL);
    for(int i = 0; i < 200; i++)
        mai_eval("big = dbl(100000) + 1.0", NULL);
    check(mai_get("big", &v) == MAI_OK && v.length == 100000 &&
          ((double *)v.data)[99999] == 1.0, "survive collections");

    mai_fini();
    printf("%s\n", fails ? "some checks failed" : "all checks passed");
    return fails != 0;
}
