#include<math.h>
#pragma once

#define LINEAR_ALL_REDUCE 0
#define RABENSEIFNER_ALL_REDUCE 1
#define RING_ALL_REDUCE 2
#define RING_SEG_ALL_REDUCE 3
#define RECURSIVE_DOUBLING_ALL_REDUCE 4

#define NUM_ALGOS 5
typedef long long ll;
typedef double (*getTimeandPc)(ll, ll, ll, double[][NUM_ALGOS+1], ll*);
//takes in P, m, alpha, beta, gamma
//pointer to Pc to return it




