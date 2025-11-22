#ifndef RECURSIVE_DOUBLING_ALLREDUCE_H
#define RECURSIVE_DOUBLING_ALLREDUCE_H

#include "macros.h"

void recursive_doubling_allreduce(void *sendbuf, void *recvbuf, ll count, MPI_Comm comm);

#endif