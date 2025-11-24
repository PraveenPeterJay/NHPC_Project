#include "ring_seg_allreduce.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void ring_seg_allreduce(void *sendbuf, void *recvbuf, ll count, MPI_Comm comm) {
    // printf("Using dummy ring Implementation instead of ring segmentation implementation\n");

    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    double *send_buf = (double*)sendbuf;
    double *recv_buf = (double*)recvbuf;
    
    int chunk_size = count / size;
    memcpy(recv_buf, send_buf, sizeof(double) * count);
    
    double* recv_chunk = (double *) malloc(sizeof(double) * chunk_size);
    
    // Reduce Scatter
    // Perform size-1 steps
    for (int step = 0; step < size - 1; step++) {
        int send_chunk_idx = (rank - step + size) % size;
        int recv_chunk_idx = (rank - step - 1 + size) % size;
        int send_to = (rank + 1) % size;
        int recv_from = (rank - 1 + size) % size;
        
        MPI_Request send_req, recv_req;
        MPI_Isend(&recv_buf[send_chunk_idx * chunk_size], chunk_size, MPI_DOUBLE,
                  send_to, 0, comm, &send_req);
        MPI_Irecv(recv_chunk, chunk_size, MPI_DOUBLE,
                  recv_from, 0, comm, &recv_req);
        
        MPI_Wait(&send_req, MPI_STATUS_IGNORE);
        MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
        
        // Reduce received chunk into result
        for (int i = 0; i < chunk_size; i++) {
            recv_buf[recv_chunk_idx * chunk_size + i] += recv_chunk[i];
        }
    }
    
    // AllGather
    // Perform size-1 steps
    for (int step = 0; step < size - 1; step++) {
        int send_chunk_idx = (rank - step + 1 + size) % size;
        int recv_chunk_idx = (rank - step + size) % size;
        int send_to = (rank + 1) % size;
        int recv_from = (rank - 1 + size) % size;
        
        MPI_Request send_req, recv_req;
        MPI_Isend(&recv_buf[send_chunk_idx * chunk_size], chunk_size, MPI_DOUBLE,
                  send_to, 1, comm, &send_req);
        MPI_Irecv(&recv_buf[recv_chunk_idx * chunk_size], chunk_size, MPI_DOUBLE,
                  recv_from, 1, comm, &recv_req);
        
        MPI_Wait(&send_req, MPI_STATUS_IGNORE);
        MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
    }
    
    free(recv_chunk);
}