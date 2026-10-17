#!/bin/sh
# Time each bench/<name>.mai against bench/<name>.R and write bench/results.csv.
# Each side defines run(); it is timed `reps` times after one warm-up call and
# the median elapsed seconds is reported.
# usage: bench/run.sh [path/to/main] [reps]
set -e
dir=$(cd "$(dirname "$0")" && pwd)
bin=${1:-"$dir/../build/main"}
reps=${2:-5}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
out="$dir/results.csv"
echo "benchmark,language,seconds" > "$out"
for m in "$dir"/*.mai; do
    name=$(basename "$m" .mai)
    # mai: one warm-up, then print one elapsed time per line
    { cat "$m"; cat <<MAI
srand(1)
run()
for(int k = 1; k <= $reps; k = k + 1) {
    var t0 = rt_time()
    run()
    print((rt_time() - t0)[1])
}
MAI
    } > "$tmp/b.mai"
    "$bin" "$tmp/b.mai" | grep -E '^[0-9.]+$' | sort -n |
        awk -v n="$name" '{ a[NR] = $1 } END { print n ",mai," a[int((NR + 1) / 2)] }' >> "$out"
    # R: same protocol
    Rscript -e "set.seed(1); source('$dir/$name.R'); invisible(run());
        t <- replicate($reps, system.time(run())[['elapsed']]);
        cat('$name,R,', median(t), '\n', sep = '')" >> "$out"
    echo "done $name" >&2
done
cat "$out"
