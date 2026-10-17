# Worked examples

Each example is a short, commented mai script in
[`../examples`](../examples), with a matching `.out` file holding its exact
output. `make check` runs them all, so they stay in step with the build.

```sh
build/main examples/01-basics.mai
```

| Example | What it shows |
| --- | --- |
| [01-basics](../examples/01-basics.mai) | Scalars, vectors and printing; what a script prints |
| [02-functions](../examples/02-functions.mai) | Functions as values, named and default arguments, a small typed `map` |
| [03-types](../examples/03-types.mai) | Optional type declarations and how they are checked |
| [04-arrays](../examples/04-arrays.mai) | Column-major arrays made with `array(type, shape, fill)` |
| [05-monte-carlo-pi](../examples/05-monte-carlo-pi.mai) | Estimating pi with a plain loop that needs no vectorising |
| [06-newton](../examples/06-newton.mai) | Newton's method with a `while` loop and a tolerance |
| [07-regression](../examples/07-regression.mai) | Least squares by modified Gram-Schmidt (thesis section 1.2.1) |
| [08-references](../examples/08-references.mai) | Vectors passed by reference, and copying when you need to |

The thesis's own examples are in `src/*.src`: `fib.src`, `regression.src`
and `closest-points.src`. The last one is a long-running benchmark.
