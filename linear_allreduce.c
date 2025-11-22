#include "linear_allreduce.h"
#include <string.h>
#include <stdlib.h>

void linear_allreduce(void *sendbuf, void *recvbuf, ll count, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    double *send_buf = (double*)sendbuf;
    double *recv_buf = (double*)recvbuf;
    const int ROOT = 0;
    
    double *temp_buf = (double*)malloc(count * sizeof(double));
    memcpy(recv_buf, send_buf, count * sizeof(double));

    // PHASE 1: REDUCE TO ROOT
    for (int i = size - 1; i > ROOT; i--) {
        if (rank == i) {
            MPI_Send(recv_buf, count, MPI_DOUBLE, rank - 1, 0, comm); 
        } else if (rank == i - 1) {
            MPI_Recv(temp_buf, count, MPI_DOUBLE, rank + 1, 0, comm, MPI_STATUS_IGNORE);
            for (ll j = 0; j < count; j++) {
                recv_buf[j] += temp_buf[j];
            }
        }
    }
    
    MPI_Barrier(comm); 

    // PHASE 2: BROADCAST FROM ROOT
    for (int i = 0; i < size - 1; i++) {
        if (rank == i) {
            MPI_Send(recv_buf, count, MPI_DOUBLE, rank + 1, 1, comm);
        } else if (rank == i + 1) {
            MPI_Recv(recv_buf, count, MPI_DOUBLE, rank - 1, 1, comm, MPI_STATUS_IGNORE);
        }
    }

    free(temp_buf);
}