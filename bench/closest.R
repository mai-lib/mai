# nearest neighbour by explicit double loop (thesis section 1.2.2)
eucl.dist <- function(x, y) sqrt(sum((x - y) * (x - y)))
closest <- function(xs, ys) {
    res <- integer(ncol(xs))
    for (i in seq_len(ncol(xs))) {
        idx <- -1; d <- Inf; x <- xs[, i]
        for (j in seq_len(ncol(ys))) {
            r <- eucl.dist(x, ys[, j])
            if (r < d) { idx <- j; d <- r }
        }
        res[i] <- idx
    }
    res
}
xa <- matrix(rnorm(100 * 500), 100); ya <- matrix(rnorm(100 * 500), 100)
run <- function() closest(xa, ya)
