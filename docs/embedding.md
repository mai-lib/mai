# Embedding mai

mai can be built as a C library, so that another program (an R package, a
Python module, a notebook kernel) can run mai code in its own process and
pass data in and out without going through files or text.

```sh
make lib            # builds build/libmai.so and build/libmai.a
```

The interface is one header, [`src/mai.h`](../src/mai.h). It uses only C
types, and the shared library exports only the functions it declares.
The design and the implementation are written up in `embed.nw`, in the
same literate style as the rest of the thesis.

## A complete example

```c
#include <stdio.h>
#include "mai.h"

int main(void)
{
    double xs[] = { 1.5, 2.5, 3.0 };
    mai_value_t v = { MAI_DOUBLE, 3, 1, NULL, xs };
    char *out;

    mai_init();
    mai_set("xs", &v);                   /* copy a C array into global xs */
    if(mai_eval("ys = xs * 2\nsum(ys)", &out) == MAI_OK)
        printf("%s\n", out);             /* 14.0 */
    mai_free(out);
    if(mai_get("ys", &v) == MAI_OK)      /* borrow ys's elements */
        printf("%g\n", ((double *)v.data)[2]);   /* 6 */
    mai_fini();
    return 0;
}
```

```sh
cc -Isrc example.c build/libmai.a -lm -lreadline
```

## Functions

| Function | What it does |
| --- | --- |
| `mai_init()` | Start the runtime. Safe to call more than once; `mai_eval` calls it for you. |
| `mai_fini()` | Shut the runtime down. |
| `mai_eval(code, &out)` | Parse, compile and run `code` at top level, as if typed at the REPL, so definitions persist between calls. `out` (may be `NULL`) receives the printed value of the last expression, or the error messages; release it with `mai_free`. |
| `mai_get(name, &val)` | Describe a global's value without copying it. |
| `mai_set(name, &val)` | Copy a buffer into a global, creating it if need be. |
| `mai_free(ptr)` | Release a string returned by `mai_eval`. |

`mai_eval` returns `MAI_OK`, `MAI_EPARSE` (syntax or compile error) or
`MAI_ERUN` (run-time error). `mai_get` and `mai_set` return `MAI_OK` or
`MAI_EARG` (no such global, a `const` global, or a value that doesn't fit
the global's declared type).

## Values

A `mai_value_t` is a flat buffer plus a shape:

| Field | Meaning |
| --- | --- |
| `type` | `MAI_BOOLEAN` (one byte), `MAI_INT` (32-bit), `MAI_DOUBLE`, or `MAI_OTHER` for anything else, such as a closure |
| `length` | number of elements |
| `rank` | 0 for a scalar, 1 for a vector, 2 or more for an array |
| `shape` | the extents of an array, `NULL` otherwise |
| `data` | the elements; arrays are column-major, as in R |

The buffer from `mai_get` belongs to mai. It stays valid only until the
next `mai_eval` or `mai_set`, either of which can let the garbage collector
reclaim it, so copy it if you need to keep it.

## Limits

- There is one interpreter per process. The globals, the collector and the
  VM stack are process-wide, and none of it is thread-safe: call into the
  library from one thread at a time.
- Output from mai's `print()` goes straight to the process's standard
  output, not into `out`.
- An internal error the runtime can't recover from calls `abort()`, which
  ends the host process too. Ordinary errors in mai code are returned.
- Like the executable, the library builds on Linux with glibc only, and
  links GNU readline. It is GPL-2, so a program that links it must be
  distributed under GPL-2-compatible terms.

## Tests

`make check` builds the library and runs [`tests/embed`](../tests/embed),
which exercises every function in `mai.h`.
