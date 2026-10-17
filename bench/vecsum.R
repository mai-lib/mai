# vectorised arithmetic over a long vector (R's home ground)
x <- runif(5e6)
run <- function() sum(sqrt(x * x + 1))
