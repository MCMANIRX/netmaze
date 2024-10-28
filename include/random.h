//random generator from my 80s genetic algorithms textbook
#include <stdint.h>
#include <stdio.h>
#include <math.h>


double oldrand[55];
int jrand;

double current_random; 

void advance_random()
{

    int jl;
    double new_random;

    for (jl = 1; jl < 24; ++jl)
    {
        new_random = oldrand[jl] - oldrand[jl + 31];
        if (new_random < 0.0)
            new_random += 1.0;
        oldrand[jl] = new_random;
    }

    for (jl = 25; jl < 55; ++jl)
    {
        new_random = oldrand[jl] - oldrand[jl - 24];
        if (new_random < 0.0)
            new_random += 1.0;
        oldrand[jl] = new_random;
    }
}

void warmup_random(double seed)
{

    int j1, ii;
    double new_random, prev_random;

    oldrand[55] = seed;
    new_random = 1E-9;
    prev_random = seed;

    for (j1 = 1; j1 < 54; ++j1)
    {

        ii = (21 * j1) % 55;

        oldrand[ii] = new_random;
        new_random = prev_random - new_random;

        if (new_random < 0.0)
            new_random += 1.0;

        prev_random = oldrand[ii];
    }

    for (int x = 0; x < 3; ++x)
        advance_random();

    jrand = 0;
}

double __random()
{

    jrand += 1;
    if (jrand > 55)
    {
        jrand = 1;
        advance_random();
    }
    current_random = oldrand[jrand];
    return current_random;
}

