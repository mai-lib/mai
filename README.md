# mai

[![CI](https://github.com/mai-lib/mai/actions/workflows/ci.yml/badge.svg)](https://github.com/mai-lib/mai/actions/workflows/ci.yml)
[![Docs](https://img.shields.io/badge/docs-mai--lib.github.io%2Fmai-blue)](https://mai-lib.github.io/mai/)

**mai** is a compiled language for statistical computing. It looks a lot like R,
but it is designed so that an optimising compiler can make ordinary loop-heavy
code fast.

The language, compiler and virtual machine are the work of Brendan McArdle, and
are described in full in his PhD thesis:

> Brendan McArdle (2018). *A Compiled Language for Statistical Computing.*
> PhD thesis, Department of Statistics, The University of Auckland.
> <https://researchspace.auckland.ac.nz/items/9dd448b6-5736-40f8-b9c8-e2aedff3d385>

The thesis did not give the language a name. "mai" stands for McArdle And
Ihaka, after the author and Ross Ihaka (co-creator of R, whose design runs
through this work). It is also a nod to `par("mai")`, and to te reo Maori
*mai*, "towards / hither". This repository hosts the thesis source so that it
is easy to find, build, document and extend.

## What it is

- A dynamically typed language with lexical scope, first-class functions and
  R-like syntax, plus optional type declarations (`double x`, `: int`),
  `const` globals, and `var` / `let` locals.
- Scalars are distinct from vectors. Vectors and arrays are passed by
  reference rather than copied.
- An optimising compiler working on an SSA intermediate representation:
  type recovery, inlining, constant folding, dead code elimination.
- A directly threaded virtual machine with unboxed scalar arithmetic, and a
  run-time library with garbage-collected vectors, arrays and closures.

## Build

You need Linux with glibc, a C11 compiler (GCC or Clang), GNU Bison 3, GNU
M4, sed, GNU readline, and the valgrind headers (the garbage collector
annotates memory for valgrind).

```sh
# Debian / Ubuntu
sudo apt-get install build-essential bison m4 libreadline-dev valgrind

make build          # produces build/main
make check          # runs the smoke tests in tests/
```

By default the build is tuned for the machine it runs on (`-march=native`),
as in the thesis. For a binary that runs on other machines of the same
architecture, use:

```sh
make build CTUNE=
```

x86-64 Linux is the tested platform. Linux arm64 is tried in CI. macOS does
not build yet, because the runtime includes glibc's `<mcheck.h>`.

You don't need noweb to build. If it is installed, `make` re-extracts `src/`
from the `.nw` files whenever they look newer.

## Use

```sh
build/main script.mai    # run a file and print the value of its last expression
build/main               # start the interactive REPL
```

To run mai from another program, `make lib` builds `build/libmai.so` and
`build/libmai.a`; see [Embedding](docs/embedding.md). From Python,
`pip install ./python` gives you the [pymai](python/README.md) package and a
`%%mai` Jupyter cell magic. From R, use [rmai](https://github.com/mai-lib/rmai).

When it starts without a file, the REPL loads `~/.maininit` if it exists
(override this with the `INITFILE` environment variable).

A first taste, from section 1.2 of the thesis:

```
const fib_rec = function(n)
    if(n <= 2) 1 else fib_rec(n - 2) + fib_rec(n - 1)

const fib.arith = function(n) {
    var sqrt5 = sqrt(5.0),
        phi = (1 + sqrt5) / 2
    as_int((phi ^ n - (1 - phi) ^ n) / sqrt5)
}

print(fib_rec(25))       # 75025
print(fib.arith(1:12))   # vector(int) [12]  1 1 2 3 5 8 13 21 34 55 89 144
```

Some differences from R that catch you out early:

- the logical constants are `true` and `false`, not `TRUE` / `FALSE`;
- there is no `c()`: build vectors with `vec(...)`, `1:n`, or `dbl(n)` /
  `int(n)` for zero-filled storage;
- `for` loops are C-style: `for(i = 1; i <= n; i = i + 1)`;
- arguments are evaluated eagerly (no lazy evaluation or `substitute`).

See the [documentation](https://mai-lib.github.io/mai/) (also readable
[here in `docs/`](docs/README.md)) for a full guide, a
[differences from R](docs/differences-from-r.md) table and a
[built-in function reference](docs/builtins.md). There are
[worked examples](examples) in `examples/`, and the thesis's own in `src/*.src`.

## Performance

Loop-heavy code runs 2.5 to 3.5 times faster than the same code in R.
Long vectorised expressions are slower than R's built-ins. See
[Performance](docs/performance.md) for details and how to reproduce the
numbers.

![mai vs R speedup per benchmark](docs/images/benchmarks.png)

## Repository layout

| Path | What |
| --- | --- |
| `*.nw` | the thesis itself as noweb literate source: prose and code together |
| `src/` | C, Bison and M4 sources extracted from the `.nw` files |
| `deps/` | generated dependency fragments so that `make build` works without noweb |
| `Makefile`, `*.mk` | the original build system |
| `tests/` | smoke tests: `*.mai` scripts with expected `*.out` output, and `tests/embed` for the library |
| `examples/` | worked examples, each with its expected output (also run by `make check`) |
| `docs/` | the documentation |
| `bench/` | benchmarks against R |
| `python/` | the pymai Python package |
| `README` | the original build notes |

The first commit on `main` is the thesis tarball, unmodified. Every change
since then is in the git history. If you change code, note that `src/` is
generated from the `.nw` files: `make distclean` deletes `src/`, and
rebuilding it needs [noweb](https://www.cs.tufts.edu/~nr/noweb/).

## Licence

Section 1.6 of the thesis places its source code under the GNU General Public
License, version 2 (see `COPYING`), unless stated otherwise. It includes:

- MT19937-64 by Makoto Matsumoto and Takuji Nishimura, under a BSD-style
  licence;
- portions of Mathlib (Copyright (C) 1998 Ross Ihaka, (C) 2000-9 The R Core
  Team), under GPL-2;
- code from the Linux kernel and the Mesa 3D Graphics Library, under GPL-2.

The thesis PDF itself is covered by the University of Auckland's thesis
copyright terms. Read it via the ResearchSpace link above.
