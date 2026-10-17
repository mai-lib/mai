#!/bin/sh
# Build the embedding test against build/libmai.a and run it.
# usage: tests/embed/run.sh [builddir]   (default build)
set -e
dir=$(cd "$(dirname "$0")" && pwd)
build=$(cd "${1:-$dir/../../build}" && pwd)
${CC:-cc} -std=c11 -I"$dir/../../src" -o "$build/embed-test" \
    "$dir/embed-test.c" "$build/libmai.a" -lm -lreadline
"$build/embed-test"
