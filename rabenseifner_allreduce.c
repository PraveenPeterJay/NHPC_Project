#include "rabenseifner_allreduce.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void rabenseifner_allreduce(void *sendbuf, void *recvbuf, ll count, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    // Check power of 2
    if ((size & (size-1)) != 0) {
        if (rank == 0) {
            fprintf(stderr, "Rabenseifner requires power-of-2 processes\n");
        }
        MPI_Abort(comm, 1);
    }

    double *send_buf = (double*)sendbuf;
    double *recv_buf = (double*)recvbuf;
    
    memcpy(recv_buf, send_buf, count * sizeof(double));
    double *tempbuf = (double*)malloc(count * sizeof(double));

    ll recv_size = count;
    ll recv_offset = 0;
    ll send_offset = 0;
    int mask = 1;

    // PHASE 1: REDUCE-SCATTER
    while(mask < size) {
        int partner = rank ^ mask;
        recv_size /= 2;

        if (rank < partner) {
            send_offset = recv_offset + recv_size;
            
            MPI_Request send_req, recv_req;
            MPI_Irecv(tempbuf, recv_size, MPI_DOUBLE, partner, 0, comm, &recv_req);
            MPI_Isend(&recv_buf[send_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &send_req);
            MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
            MPI_Wait(&send_req, MPI_STATUS_IGNORE);

            for (ll i = 0; i < recv_size; i++) {
                recv_buf[recv_offset+i] += tempbuf[i];
            }
        } else {
            send_offset = recv_offset;
            recv_offset += recv_size;
            
            MPI_Request send_req, recv_req;
            MPI_Irecv(tempbuf, recv_size, MPI_DOUBLE, partner, 0, comm, &recv_req);
            MPI_Isend(&recv_buf[send_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &send_req);
            MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
            MPI_Wait(&send_req, MPI_STATUS_IGNORE);

            for (ll i = 0; i < recv_size; i++) {
                recv_buf[recv_offset+i] += tempbuf[i];
            }
        }
        mask *= 2;
    }

    send_offset = recv_offset;
    mask = size / 2;

    // PHASE 2: ALLGATHER
    while(mask > 0) {
        int partner = rank ^ mask;

        if (rank < partner) {
            send_offset = recv_offset;
            ll partnerValue_offset = recv_offset + recv_size;
            
            MPI_Request send_req, recv_req;
            MPI_Irecv(&recv_buf[partnerValue_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &recv_req);
            MPI_Isend(&recv_buf[send_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &send_req);
            MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
            MPI_Wait(&send_req, MPI_STATUS_IGNORE);
        } else {
            send_offset = recv_offset;
            ll partnerValue_offset = recv_offset - recv_size;
            
            MPI_Request send_req, recv_req;
            MPI_Irecv(&recv_buf[partnerValue_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &recv_req);
            MPI_Isend(&recv_buf[send_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &send_req);
            MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
            MPI_Wait(&send_req, MPI_STATUS_IGNORE);

            recv_offset = partnerValue_offset;
        }

        recv_size *= 2;
        mask /= 2;
    }

    free(tempbuf);
}