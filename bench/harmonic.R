# scalar loop: sum of 1/i for i = 1..n
harmonic <- function(n) {
    s <- 0
    for (i in 1:n) s <- s + 1 / i
    s
}
run <- function() harmonic(1e7)
