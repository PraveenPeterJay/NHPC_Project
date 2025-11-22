#ifndef RING_SEG_ALLREDUCE_H
#define RING_SEG_ALLREDUCE_H

#include "macros.h"

void ring_seg_allreduce(void *sendbuf, void *recvbuf, ll count, MPI_Comm comm);

#endif