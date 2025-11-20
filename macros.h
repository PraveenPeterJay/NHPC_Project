#include<math.h>
#include<mpi.h>

#pragma once

#define LINEAR_ALL_REDUCE 0
#define RABENSEIFNER_ALL_REDUCE 1
#define RING_ALL_REDUCE 2
#define RING_SEG_ALL_REDUCE 3
#define RECURSIVE_DOUBLING_ALL_REDUCE 4


#define NUM_ALGOS 5
#define MAX_FACTORS (int)1e5
// int MAX_FACTORS = (int)1e5;

typedef long long ll;

typedef double (*getTimeandPc)(ll, ll, ll, double[][NUM_ALGOS+1], ll*);
//takes in P, m, alpha, beta, gamma
//pointer to Pc to return it

typedef void (*execAllReduce)(void *, void *, ll, MPI_Comm);
//int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)

extern double times41[NUM_ALGOS+1][NUM_ALGOS+1]; 


void find_and_store_factors(int P);
extern int NUM_FACTORS;
extern int factorsP[MAX_FACTORS+1];


