dotprod = function(x, y) sum(x * y)
norm = function(x) sqrt(dotprod(x, x))
colscl = function(a, x) a * x
colswp = function(y, a, x) y - a * x
reg = function(X, y, eps = 1e-7) {
    R = diag(ncol(X))
    b = rep(0, ncol(R))
    n = nrow(X)
    p = ncol(X)
    xnorm = numeric(p)
    s = 0
    for(j in 1:p)
        xnorm[j] = norm(X[,j])
    for(j in 1:p) {
        s = norm(X[,j])
        if (s/xnorm[j] < eps)
            stop("singular X matrix")
        s = 1/s
        X[,j] = colscl(s, X[,j])
        R[,j] = colscl(s, R[,j])
        if (j < p)
            for(k in (j+1):p) {
                s = dotprod(X[,j], X[,k])
                X[,k] = colswp(X[,k], s, X[,j])
                R[,k] = colswp(R[,k], s, R[,j])
            }
        s = dotprod(X[,j], y)
        y = colswp(y, s, X[,j])
        b = colswp(b, -s, R[,j])
    }
    list(Q = X, residuals = y, Rinv = R, coeff = b)
}
n = 10000
k = 100
X = cbind(replicate(k/2,rnorm(n)+1:n), replicate(k/2,rnorm(n)))
y = 1 + 1:n + .1 * rnorm(n)
mean(replicate(10, system.time(reg(X, y))[["elapsed"]]))
