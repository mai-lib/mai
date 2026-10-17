# Getting started

## Build

mai builds on Linux with glibc. You need a C11 compiler, GNU Bison 3, GNU M4,
GNU readline and the valgrind headers.

```sh
sudo apt-get install build-essential bison m4 libreadline-dev valgrind
git clone https://github.com/mai-lib/mai
cd mai
make build          # tuned for this machine (-march=native)
make check          # runs tests/ and examples/
```

`make build CTUNE=` builds a binary that isn't tied to your CPU model, which
is what CI does. The executable is `build/main`; copy or symlink it anywhere
you like, for example `ln -s "$PWD/build/main" ~/bin/mai`.

## Run a script

```sh
build/main examples/01-basics.mai
```

A script runs top to bottom. Output comes from `print()`, and the value of
the script's final expression is printed at the end. That is why most
example outputs end in `<nil>`: `print()` itself returns `nil`.

Errors are printed with the name of the function that raised them, for
example `stop: singular X matrix`. The process still exits with status 0,
so check the output rather than the exit code.

## The REPL

Run `build/main` with no arguments to get an interactive prompt (`> `, and
`+ ` for continuation lines) with readline editing and history.

When the REPL starts it loads `~/.maininit` if that file exists. Set the
`INITFILE` environment variable to use a different file. That makes it a
good place for helper functions you always want, such as `mean` or `map`.

## Splitting code across files

- `include "file.mai"` reads another file in place, at compile time. The
  path is relative to the current directory.
- `source("file.mai")` compiles a file at run time and returns it as a
  function of no arguments, so `source("file.mai")()` runs it.

## Options

`options()` returns the current settings, and `options(name = value)`
changes them:

| Option | Default | Effect |
| --- | --- | --- |
| `opt_builtin` | `true` | specialise calls to built-in functions |
| `opt_inline` | `true` | inline small `const` functions |
| `opt_dce` | `true` | dead code elimination |
| `opt_constfold` | `true` | constant folding |
| `opt_dvn` | `true` | redundancy elimination (dominator-based value numbering) |
| `dbg_dump_ir` | `false` | print the IR and bytecode of each newly compiled function |
| `print_digits` | `6` | significant digits when printing doubles |
| `print_line` | `76` | line width for printing vectors |
| `print_max` | `200` | maximum number of elements printed |
| `print_name_max` | `15` | maximum width of printed names |

An option takes effect for code compiled after the call. Code in the same
file was compiled before the call ran, so use `include` or `source()` to
see its effect, as in the IR dump example in [Internals](internals.md).
