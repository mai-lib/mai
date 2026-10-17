# Differences from R

mai keeps much of R's syntax and changes the semantics that stop R from being
compiled well (thesis section 1.1). This table is what R users hit first.

| R | mai | Notes |
| --- | --- | --- |
| `TRUE`, `FALSE` | `true`, `false` | |
| `x <- 1` | `x = 1` | there is no `<-`, `<<-` or `->` |
| `c(1, 2, 3)` | `vec(1, 2, 3)` | `cat(a, b)` concatenates vectors |
| `numeric(n)`, `integer(n)`, `logical(n)` | `dbl(n)`, `int(n)`, `bool(n)` | `vector(type(double), n, fill)` for an explicit fill |
| `list(a, b)` | `vec(a, b)`, or `list(n)` then assign | `list(n)` makes `n` empty slots |
| `for (i in 1:n)` | `for(i = 1; i <= n; i = i + 1)` | C-style only; the loop variable is local to the loop |
| `7 / 2` is `3.5` | `7 / 2` is `3` | int / int stays int; write `7 / 2.0` |
| `2^31` is a double | `2 ^ 31` is `NA` | int overflow gives `NA`; write `2.0 ^ 31` |
| `x %% y`, `x %/% y` | not available | |
| `as.integer`, `as.numeric` | `as_int`, `as_double`, `as_bool`, `as_type(x, type(T))` | |
| `is.na(x)` | `is_na(x)` | |
| scalars are length-1 vectors | scalars are distinct types | `typeof(1)` is `int`, `typeof(1:1)` is `vector(int)` |
| copy-on-modify | **call by reference** | a function that assigns into a vector argument changes the caller's vector |
| assignment in a function makes a local | assignment to an undeclared name sets a **global** | declare locals with `var` or `let` |
| lazy arguments, `substitute()` | eager arguments | no non-standard evaluation |
| `sum(x, na.rm = TRUE)` | no `na.rm` | |
| `system.time(expr)` | `rt_time()` before and after | |
| `set.seed(1)` | `srand(1)` | the generator is MT19937-64, so streams differ from R's |
| `source("f.R")` | `include "f.mai"` or `source("f.mai")()` | `source()` returns the compiled file as a function |
| `library(pkg)` | not available | no packages, S3/S4 or data frames yet |
| `lapply`, `sapply`, `apply` | write a loop, or a small typed `map` | see [`examples/02-functions.mai`](../examples/02-functions.mai) |
| `mean`, `var`, `colSums`, `which.min` | not built in | a few lines of mai each; the thesis's closest-points example defines `which.min` and `colsum` this way |
| errors give a non-zero exit in `Rscript` | the process exits 0 | check the output |

## What carries over unchanged

- `if`/`else` and `{ }` blocks as expressions; the last value is returned.
- First-class functions and closures, with default and named arguments.
- Element-wise, recycling arithmetic and comparison on vectors.
- `a:b` ranges, 1-based indexing, `x[i]`, `m[i, j]`, `m[, j]`, names.
- `NA` propagation.
