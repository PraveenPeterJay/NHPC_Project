#ifndef RABENSEIFNER_ALLREDUCE_H
#define RABENSEIFNER_ALLREDUCE_H

#include <mpi.h>

void custom_rabenseifner_allreduce_p2p(double *send_buf, double *recv_buf, int count, MPI_Comm comm);   // function you want to export

#endif