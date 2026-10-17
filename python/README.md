# pymai

Run [mai](https://github.com/mai-lib/mai), the compiled statistical language
from Brendan McArdle's thesis, from Python and Jupyter.

pymai loads `libmai` (see [Embedding](../docs/embedding.md)) into the
Python process. Definitions persist from one call to the next, and numpy
arrays go in and out without a text round trip.

## Install

From a checkout of the mai repository, on Linux with the build tools mai
needs (see the [main README](../README.md#build)):

```sh
pip install ./python
```

This runs `make lib` in the repository and puts `libmai.so` inside the
package. To use a library you built yourself, set `MAI_LIB` to its path,
at install time or at run time.

## Use

```python
import numpy as np
import pymai

pymai.eval("""
const total = function(vector(double) x) : double {
    var s = 0.0
    for(i = 1; i <= length(x); i = i + 1) s = s + x[i]
    s
}
""")
pymai.set("v", np.arange(1, 1001, dtype=float))
pymai.eval("total(v)")          # '500500.0'
pymai.eval("w = v * 2")
pymai.get("w")[:3]              # array([2., 4., 6.])
```

| Function | What it does |
| --- | --- |
| `eval(code, capture=False)` | Run code at top level. Returns the printed value of the last expression, or `None` if it has none. With `capture=True`, returns `(value, output)`, where `output` is what the code printed. |
| `get(name)` | Copy a global out: a `bool`, `int` or `float` for a scalar, a 1-d numpy array for a vector, an n-d array for an array. |
| `set(name, value)` | Copy a Python scalar, sequence or numpy array into a global. |

Errors raise `MaiSyntaxError` or `MaiRuntimeError`, both subclasses of
`MaiError`. Element types map to mai's boolean, int (32-bit) and double.
Arrays are column-major on the mai side, and the conversion handles that,
so `m[i, j]` in mai is `m[i - 1, j - 1]` in numpy.

## Jupyter and IPython

```python
%load_ext pymai
```

```
%%mai
const fib = function(n) if(n <= 2) 1 else fib(n - 2) + fib(n - 1)
print(fib(20))
fib(25)
```

The cell prints what the code prints, then the value of the last
expression. `%mai expr` on one line returns the value's text.

## Limits

- One mai interpreter per process, shared by everything in it. pymai holds
  the GIL during calls, so threads take turns.
- Linux only for now, like mai itself.
- An internal error mai can't recover from aborts the whole process.
  Errors in mai code are raised as exceptions.
- GPL-2, like mai.

## Tests

```sh
pip install './python[test]'
pytest python/tests
```
