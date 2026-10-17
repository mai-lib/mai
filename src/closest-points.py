import gc, timeit, math, numpy as np
def eucl_dist(x, y):
    return np.sqrt(np.sum(np.power(x - y, 2)))
def closest_map_list(xs, ys):
    return [np.argmin([eucl_dist(x, y) for y in ys]) for x in xs]
def eucl_dist_vec(x, y):
    return np.sqrt(np.sum(np.power(x - y, 2), axis = 1))
def closest_map_array(xs, ys):
    return np.apply_along_axis(lambda x: np.argmin(eucl_dist_vec(x, ys)), 1, xs)
def closest_loop_list(xs, ys):
    res = np.zeros(len(xs), np.int)
    i = 0
    for x in xs:
        d = np.Inf; idx = -1; j = 0
        for y in ys:
            r = eucl_dist(x, y)
            if r < d: idx = j; d = r
            j += 1
        res[i] = idx
        i += 1
    return res
def closest_loop_array(xs, ys):
    res = np.zeros(xs.shape[0], np.int)
    for i in range(xs.shape[0]):
        d = np.Inf; idx = -1; x = xs[i,]
        for j in range(ys.shape[0]):
            r = eucl_dist(x, ys[j,])
            if r < d: idx = j; d = r
            j += 1
        res[i] = idx
        i += 1
    return res
d = 100; r = 10
def mklist(n):
    return [np.random.normal(size = d) for x in range(0, n)]
def mkarr(n):
    return np.random.normal(size = (n, d))
def test(expr):
    times = timeit.repeat(expr, 'gc.enable()', number = 1,
                          repeat = r, globals=globals())
    print(math.fsum(times) / len(times))
for n in range(1000, 3000+1, 1000):
    print(n)
    xs = mklist(n); ys = mklist(n); xa = mkarr(n); ya = mkarr(n)
    test('closest_map_list(xs,ys)')
    test('closest_loop_list(xs,ys)')
    test('closest_map_array(xa,ya)')
    test('closest_loop_array(xa,ya)')
