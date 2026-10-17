#!/bin/sh
# Run each tests/*.mai script and compare its output with the matching .out
# file. The interpreter exits 0 even on errors, so output is the oracle.
# usage: tests/run.sh [path/to/main]   (default build/main)
# set MAI_UPDATE=1 to regenerate the .out files instead of comparing.

dir=$(cd "$(dirname "$0")" && pwd)
bin=${1:-"$dir/../build/main"}
bin=$(cd "$(dirname "$bin")" && pwd)/$(basename "$bin")
# `timeout` is not on stock macOS; run without it there
to=""
command -v timeout > /dev/null 2>&1 && to="timeout 60"
pass=0
fail=0
for t in "$dir"/*.mai; do
    exp="${t%.mai}.out"
    name=$(basename "$t")
    got=$(cd "$dir" && $to "$bin" "$name" 2>&1)
    if [ -n "$MAI_UPDATE" ]; then
        printf '%s\n' "$got" > "$exp"
        echo "updated $name"
        continue
    fi
    if printf '%s\n' "$got" | diff -u "$exp" - > /dev/null; then
        echo "ok    $name"
        pass=$((pass + 1))
    else
        echo "FAIL  $name"
        printf '%s\n' "$got" | diff -u "$exp" - | sed 's/^/      /'
        fail=$((fail + 1))
    fi
done
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
