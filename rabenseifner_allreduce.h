#ifndef RABENSEIFNER_ALLREDUCE_H
#define RABENSEIFNER_ALLREDUCE_H

#include "macros.h"

void rabenseifner_allreduce(void *sendbuf, void *recvbuf, ll count, MPI_Comm comm);

#endif