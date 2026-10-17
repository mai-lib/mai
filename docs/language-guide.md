# Language guide

This is a practical tour of mai. Every snippet here has been run against the
current build. The thesis remains the authoritative reference: chapter 4
covers the grammar, and chapters 19 to 25 cover the run-time library.

## Lexical basics

- Comments run from `#` to the end of the line.
- Statements end at a newline or `;`. Inside an open `(`, `[` or `{` a
  newline continues the expression.
- Identifiers may contain `.` and `_`, as in R (`fib.arith`, `as_int`). Wrap
  any other name in backticks.
- Literals:

  | Literal | Type |
  | --- | --- |
  | `42` | int |
  | `42.0`, `1e-7`, `Inf` | double (`Inf` prints as `inf`) |
  | `true`, `false` | boolean |
  | `NA` | the missing value (it adopts the type it is used as) |
  | `"text"` | string |
  | `'name'` | symbol |
  | `type(double)`, `type(vector(int))` | type |
  | `nil` | the null object |

## Values and types

Scalars (`int`, `double`, `boolean`) are machine values, not length-one
vectors, and each has its own missing value `NA`. Everything else is an
object with reference semantics: strings, symbols, vectors, arrays, lists
and functions.

```
print(typeof(1))        # int
print(typeof(1.0))      # double
print(typeof(1:3))      # vector(int)
print(typeof(function(double x) : double x))   # function (double x) : double
```

Arithmetic keeps the type of its operands. `int` with `int` gives `int`, so
`7 / 2` is `3`, and an `int` result that overflows becomes `NA` (`2 ^ 31`).
Mixing in a `double` promotes to `double`: `7 / 2.0` is `3.5`.

Type names: `boolean`, `int`, `double`, `string`, `symbol`, `object`,
`vector`, `vector(T)`, `array`, `array(T)`, `list`, `function` and `type`.
`numeric` stands for `vector(double)`.

## Variables and declarations

Assignment uses `=` (there is no `<-`). Variables are either global or
declared local. An assignment to a name that has not been declared as a
local, **even inside a function**, creates or updates a global:

```
f = function() { y = 5; y }
f()
print(y)                    # 5: y leaked out as a global
```

So declare locals with `var` or `let`:

| Form | Meaning |
| --- | --- |
| `var x = 1, y = 2` | local variables, scoped to the function |
| `let x = 1` | local variable, scoped to the enclosing block |
| `const f = function(x) ...` | a global that can never be reassigned, so the compiler may inline it |
| `global xs, ys` | declare globals without giving them a value yet |
| `int i = 1`, `double s = 0.0` | a typed declaration (also allowed after `var` and `let`) |

```
const limit = 10
f = function() {
    var total = 0
    for(int i = 1; i <= limit; i = i + 1)
        total = total + i
    total
}
print(f())              # 55
```

Reassigning a `const` is a compile-time error: `invalid assignment to
'const k'`. Scoping is strictly lexical: there is no `assign()`, `get()` or `<<-`,
and a local is only visible in the block of code that declares it.

## Functions

```
area = function(w, h = 1) w * h
print(area(3))              # 3
print(area(h = 10, w = 2))  # 20
```

- Arguments are matched by name, then by position, and defaults may refer to
  earlier arguments.
- Arguments are evaluated before the call (no lazy evaluation, so no
  `substitute()` and no non-standard evaluation).
- A formal can carry a type, `function(double a, double b)`, and the function
  can declare a return type after the formals: `function(x) : double ...`.
  Arguments are converted to declared scalar types: passing `2.5` to an
  `int` formal gives `2`.
- `return(x)` (or `return x`) leaves early; otherwise the value of the last
  expression is returned.
- Functions are closures and capture their defining environment:

```
make_counter = function() {
    var n = 0
    function() { n = n + 1; n }
}
tick = make_counter()
tick(); tick()
print(tick())               # 3
```

## Control flow

```
if(x > 0) "positive" else "non-positive"     # if is an expression

while(i < 10) i = i + 1

for(int i = 1; i <= n; i = i + 1) {          # C-style for
    if(i == 4) break
    if(i == 2) continue
}
```

A loop variable declared in `for(...)` is local to the loop. Use `while`, or
declare the variable beforehand, if you need its final value.

## Vectors

```
v = 1:10                    # integer range
w = vec(1.5, 2.5, 3.5)      # vec() builds a vector from its arguments
named = vec(a = 1, b = 2)   # with names
z = dbl(3)                  # zero-filled: dbl(n), int(n), bool(n), list(n)
u = vector(type(double), 3, 1.5)   # explicit type, length and fill
both = cat(1:3, 4:5)        # concatenation
```

- Indexing is 1-based: `v[3]`, `v[2:4]`. `v[] = 0` fills every element.
- Arithmetic, comparison and maths functions work element-wise, recycling
  the shorter operand as in R: `1:6 * 1:2`.
- `names(x)` returns a vector of symbols, and `names(x) = vec('a', 'b')` sets them.
- `list(n)` makes a vector of `n` objects, which can hold anything.

## Arrays

Arrays are column-major, with a shape vector:

```
m = array(type(double), vec(2, 3), 0.0)
m[1, 2] = 5.0
m[, 3] = 7.0                # whole column
print(shape(m))             # 2 3
print(rank(m))              # 2
col = m[, 2]                # a column, as a vector
```

`dimnames(m)` and `dimnames(m) = ...` get and set the names of each
dimension (`nil` when unset).

## Reference semantics

Vectors and arrays are passed by reference, not copied. A function that
assigns into an argument changes the caller's data:

```
zero_first = function(v) { v[1] = 0; nil }
x = 1:5
zero_first(x)
print(x)                    # 0 2 3 4 5
```

When you need the original, pass a copy. Arithmetic always creates a new
vector, so `1 * y` or `1.0 * y` is the quickest way to copy. Scalars are
values, so reassigning a scalar argument never affects the caller.

## Missing values

`NA` exists for each scalar type. It propagates through arithmetic
(`sum(vec(1.0, NA))` is `NA`) and `is_na(x)` tests for it. Unlike R, there
is no `na.rm` argument.

## Errors

`stop("message")` raises an error. `assert(cond)` raises one if `cond` is
false. An error aborts the current top-level expression and is printed as
`function: message`.

## Randomness and timing

`srand(seed)` seeds the generator, a 64-bit Mersenne Twister. `runif(n)`,
`rnorm(n)` and `rand(min, max)` draw from it. `rt_time()` returns a
`double` vector of real, user and system seconds, so
`(rt_time() - t0)[1]` is the elapsed wall time.

## A complete example

The thesis's motivating example is a least-squares fit by modified
Gram-Schmidt. It is in [`examples/07-regression.mai`](../examples/07-regression.mai)
and benchmarked against R in [Performance](performance.md).
