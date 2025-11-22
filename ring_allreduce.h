#ifndef RING_ALLREDUCE_H
#define RING_ALLREDUCE_H

#include "macros.h"

void ring_allreduce(void *sendbuf, void *recvbuf, ll count, MPI_Comm comm);

#endif