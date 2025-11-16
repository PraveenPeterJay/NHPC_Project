#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h> 
#include <string.h> // For memcpy

#include "rabenseifner_allreduce.h"



int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // ---- TEST PARAMETERS ----
    int count = 16;   // total length of array
    double *send_buf = malloc(count * sizeof(double));
    double *recv_buf = malloc(count * sizeof(double));

    // Fill send_buf with rank-specific pattern
    for (int i = 0; i < count; i++) {
        send_buf[i] = (double)(rank+1);   // simple predictable data
    }

    if (rank == 0){
        printf("printing sendbuf contents:");
        for (int i = 0; i < count; i++) {
            printf("%f ", send_buf[i]);
        }   printf("\n");
    }

    // Call your implementation
    custom_rabenseifner_allreduce_p2p(send_buf, recv_buf, count,
                                      MPI_COMM_WORLD);

    // Print result from each rank
    MPI_Barrier(MPI_COMM_WORLD);
    printf("Rank %d result: ", rank);
    for (int i = 0; i < count; i++) {
        printf("%.1f ", recv_buf[i]);
    }
    printf("\n");
    fflush(stdout);

    free(send_buf);
    free(recv_buf);

    MPI_Finalize();
    return 0;
}
