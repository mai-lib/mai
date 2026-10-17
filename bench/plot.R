# Plot bench/results.csv as speedup of mai over R (R seconds / mai seconds).
# usage: Rscript bench/plot.R [results.csv] [out.png]
args <- commandArgs(trailingOnly = TRUE)
csv <- if (length(args) > 0) args[1] else "bench/results.csv"
png_out <- if (length(args) > 1) args[2] else "docs/images/benchmarks.png"

d <- read.csv(csv)
w <- reshape(d, idvar = "benchmark", timevar = "language", direction = "wide")
w$speedup <- w$seconds.R / w$seconds.mai
w <- w[order(w$speedup), ]

labels <- c(closest = "nearest neighbour loops",
            fib = "recursive calls (fib 27)",
            harmonic = "scalar loop (1e7 steps)",
            regression = "Gram-Schmidt regression",
            vecsum = "vectorised sum(sqrt(...))")
lab <- ifelse(w$benchmark %in% names(labels), labels[w$benchmark], w$benchmark)

surface <- "#fcfcfb"; ink <- "#0b0b0b"; ink2 <- "#52514e"
grid <- "#e4e3df"; mark <- "#2a78d6"

png(png_out, width = 1600, height = 900, res = 200, bg = surface)
op <- par(mai = c(0.9, 2.6, 0.8, 0.9), fg = ink2, col.axis = ink2,
          col.lab = ink2, family = "sans")
n <- nrow(w)
xr <- range(c(0.25, 8, w$speedup))
plot(NA, xlim = xr, ylim = c(0.5, n + 0.5), log = "x", axes = FALSE,
     xlab = "speedup over R (log scale; right of 1 means mai is faster)",
     ylab = "")
ticks <- c(0.25, 0.5, 1, 2, 4, 8)
abline(v = ticks, col = grid, lwd = 1)
abline(v = 1, col = ink2, lwd = 1.5)
axis(1, at = ticks, labels = paste0(ticks, "x"), lwd = 0, lwd.ticks = 0)
axis(2, at = seq_len(n), labels = lab, las = 1, lwd = 0, col.axis = ink)
segments(1, seq_len(n), w$speedup, seq_len(n), col = mark, lwd = 3)
points(w$speedup, seq_len(n), pch = 21, bg = mark, col = surface, cex = 1.6,
       lwd = 1.5)
text(w$speedup, seq_len(n), sprintf("%.1fx", w$speedup), col = ink,
     pos = ifelse(w$speedup >= 1, 4, 2), offset = 0.7, cex = 0.85)
title(main = "mai vs R: speedup per benchmark (median of repeated runs)", adj = 0, col.main = ink,
      font.main = 1, cex.main = 1.1, line = 1.2)
par(op)
invisible(dev.off())
cat("wrote", png_out, "\n")
