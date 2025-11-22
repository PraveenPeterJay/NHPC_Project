#include "recursive_doubling_allreduce.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


void recursive_doubling_allreduce(void *sendbuf, void *recvbuf, ll count, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    // Check power of 2
    if ((size & (size-1)) != 0) {
        if (rank == 0) {
            fprintf(stderr, "Recursive Doubling requires power-of-2 processes\n");
        }
        MPI_Abort(comm, 1);
    }
    
    double *send_buf = (double*)sendbuf;
    double *recv_buf = (double*)recvbuf;
    
    memcpy(recv_buf, send_buf, count * sizeof(double));
    double *temp_buf = (double*)malloc(count * sizeof(double));
    
    int mask = 1;
    while (mask < size) {
        int partner = rank ^ mask;
        
        MPI_Sendrecv(recv_buf, count, MPI_DOUBLE, partner, 0,
                     temp_buf, count, MPI_DOUBLE, partner, 0,
                     comm, MPI_STATUS_IGNORE);
        
        for (ll i = 0; i < count; i++) {
            recv_buf[i] += temp_buf[i];
        }
        
        mask <<= 1;
    }
    
    free(temp_buf);
}