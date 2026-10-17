# mai documentation

mai is a compiled language for statistical computing, from Brendan McArdle's
2018 PhD thesis at the University of Auckland. It reads like R but compiles
to bytecode for a fast virtual machine.

| Page | What's in it |
| --- | --- |
| [Getting started](getting-started.md) | Build, run a script, use the REPL, split code across files |
| [Language guide](language-guide.md) | Values, types, declarations, functions, control flow, vectors and arrays |
| [Differences from R](differences-from-r.md) | What an R user trips over first, in one table |
| [Built-in functions](builtins.md) | Every built-in, grouped by purpose |
| [Performance](performance.md) | Benchmarks against R, and how to reproduce them |
| [Embedding](embedding.md) | Run mai from C, R or Python through `libmai` and `mai.h` |
| [Internals](internals.md) | How the compiler and VM fit together, and where each part lives in the thesis |

Worked examples live in [`../examples`](../examples). Each `.mai` script has
a matching `.out` file holding its exact output, and `make check` runs them
all. So the examples are tested along with everything else.

![mai vs R speedup per benchmark](images/benchmarks.png)
