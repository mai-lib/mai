eucl.dist = function(x, y)
    sqrt(sum((x - y) ^ 2))
closest.map.list = function(xs, ys)
    lapply(xs, function(x) which.min(lapply(ys, function(y) eucl.dist(x,y))))
eucl.dist.vec = function(x, ys)
    sqrt(colSums((x - ys) ^ 2))
closest.map.array = function(xs, ys)
    apply(xs, 2, function(x) which.min(eucl.dist.vec(x, ys)))
closest.loop.list = function(xs, ys) {
    res = integer(length(xs))
    i = 1
    for(x in xs) {
        d = Inf; idx = -1; j = 1
        for(y in ys) {
            r = eucl.dist(x, y)
            if(r < d) { idx = j; d = r; }
            j = j + 1
        }
        res[i] = idx
        i = i + 1
    }
    res
}
closest.loop.array = function(xs, ys) {
    res = integer(ncol(xs))
    i = 1
    while(i <= ncol(xs)) {
        d = Inf; yn = -1; j = 1; x = xs[,i]
        while(j <= ncol(ys)) {
            r = eucl.dist(x, ys[,j])
            if(r < d) { idx = j; d = r; }
            j = j + 1
        }
        res[i] = idx
        i = i + 1
    }
    res
}
d = 100; r = 10
prep = function(n, arr = FALSE)
    replicate(n, rnorm(d), simplify = arr)
test = function(fn)
    cat(mean(replicate(r, system.time(fn())[["elapsed"]])), "\n")
for(n in seq(1000, 3000, 1000)) {
    cat(n, "\n")
    xs = prep(n); ys = prep(n); xa = prep(n, T); ya = prep(n, T)
    test(function() closest.map.list(xs, ys))
    test(function() closest.loop.list(xs, ys))
    test(function() closest.map.array(xa, ya))
    test(function() closest.loop.array(xa, ya))
}
