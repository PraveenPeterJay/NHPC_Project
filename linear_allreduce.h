#ifndef LINEAR_ALLREDUCE_H
#define LINEAR_ALLREDUCE_H

#include "macros.h"

void linear_allreduce(void *sendbuf, void *recvbuf, ll count, MPI_Comm comm);

#endif