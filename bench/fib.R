# recursive function calls
fib <- function(n) if (n <= 2) 1 else fib(n - 2) + fib(n - 1)
run <- function() fib(27)
