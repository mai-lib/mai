# Built-in functions

These are all the functions the run-time provides (`src/rt/builtins.c`,
`src/arith/builtins.c`, `src/rt/options.c`). Anything else, such as `mean`,
`map` or `which.min`, is ordinary mai code that you write or `include`.

## Arithmetic and comparison operators

`+ - * / ^`, unary `-` and `!`, `& |` (element-wise boolean), `&& ||`,
`== != < <= > >=`, and `a:b` (integer range). All are element-wise on
vectors and arrays and recycle the shorter operand.

## Maths (element-wise)

| Function | |
| --- | --- |
| `sqrt`, `exp`, `log` | |
| `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2(y, x)` | |
| `round` | rounds to the nearest whole number, returns double |
| `is_na` | |

## Reductions

`sum`, `prod`, `any`, `all`.

## Construction and conversion

| Function | |
| --- | --- |
| `vec(...)` | vector from the arguments, optionally named: `vec(a = 1, b = 2)` |
| `vector(etyp, len, val)` | vector of type `etyp` and length `len`, filled with `val` |
| `array(etyp, shape, val)` | array of type `etyp` with the given shape vector |
| `dbl(n)`, `int(n)`, `bool(n)`, `list(n)` | zero-filled vectors (`list` holds objects) |
| `cat(x, y)` | concatenation |
| `as_double`, `as_int`, `as_bool` | element-wise conversion (`as_int` truncates) |
| `as_type(x, type(T))` | convert to any type |

## Structure

| Function | |
| --- | --- |
| `length(x)` | number of elements |
| `shape(a)`, `rank(a)` | dimensions and number of dimensions of an array |
| `names(x)`, `names(x) = syms` | element names, as a vector of symbols |
| `dimnames(a)`, `dimnames(a) = v` | per-dimension names of an array |

## Types

| Function | |
| --- | --- |
| `typeof(x)` | the type of a value |
| `ret_type(t)`, `elt_type(t)` | return type of a function type; element type of a vector or array type |
| `common_type(a, b)` | the type both promote to: `common_type(type(int), type(double))` is `double` |
| `subtype(a, b)`, `compat(a, b)` | type relations, as used by the compiler |

## Random numbers

`srand(seed)`, `runif(n)`, `rnorm(n)` and `rand(min, max)` (a random int),
all from MT19937-64.

## Data exchange

| Function | |
| --- | --- |
| `read_vec(file, type(T))` | read a file of raw `T` elements (`int` as 32-bit, `double` as 64-bit IEEE, `boolean` as one byte, native byte order) into a `vector(T)` |
| `write_vec(x, file)` | write the elements of an `int`, `double` or `boolean` vector or array the same way, returning the count; arrays are written column-major |

These are how [rmai](https://github.com/mai-lib/rmai) passes data between R
and mai exactly. From R, `writeBin(x, f, size = 8)` and
`readBin(f, "double", n)` speak the same format.

## Program and environment

| Function | |
| --- | --- |
| `print(x)` | print and return `nil` |
| `stop(message)` | raise an error |
| `assert(cond)` | raise an error unless `cond` is true |
| `source(file)` | compile a file and return it as a function |
| `rm('name')` | remove a global |
| `options(...)` | get or set compiler and printing options (see [Getting started](getting-started.md#options)) |
| `rt_time()` | real, user and system seconds since an arbitrary origin |
| `gc_collect()` | run the garbage collector |
| `eq(x, y)` | identity of two objects |
| `equal(x, y)` | equality of contents |
| `hash(x)` | hash of a value |

Keywords that look like functions: `if`, `while`, `for`, `function`,
`return`, `break`, `continue`, `type(...)`, `include "file"`, and the
declarations `var`, `let`, `const` and `global`.
