/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998   Ross Ihaka
 *  Copyright (C) 2000-9 The R Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  https://www.R-project.org/Licenses/
 *
*/

#include "global.h"

// mersenne twister

void init_genrand64(unsigned long long seed);
unsigned long long genrand64_int64(void);
double genrand64_real3(void);

void init_rand(unsigned long long seed)
{
    init_genrand64(seed);
}

double unif_rand()
{
    return genrand64_real3();
}

unsigned unif_rand_u32()
{
    return (unsigned)genrand64_int64();
}

// box-muller rng from MathLib

static double BM_norm_keep = 0.0;

double norm_rand()
{
    if(BM_norm_keep != 0.0)
    { /* An exact test is intentional */
        double s = BM_norm_keep;
        BM_norm_keep = 0.0;
        return s;
    }

    double theta = 2 * M_PI * unif_rand();
    double R = sqrt(-2 * log(unif_rand())) + 10*DBL_MIN; /* ensure non-zero */
    BM_norm_keep = R * sin(theta);
    return R * cos(theta);
}

double rnorm(double mu, double sigma)
{
    if(isnan(mu) || !isfinite(sigma) || sigma < 0)
        return NAN;
    if(sigma == 0 || !isfinite(mu))
        return mu;
    return mu + sigma * norm_rand();
}
