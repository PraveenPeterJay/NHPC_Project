#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "linear_allreduce.h"
#include "rabenseifner_allreduce.h"
#include "ring_allreduce.h"
#include "ring_seg_allreduce.h"
#include "recursive_doubling_allreduce.h"

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int m = 16;
    double *sendbuf = (double*)malloc(m * sizeof(double));
    double *recvbuf = (double*)malloc(m * sizeof(double));
    double expected = size * (size + 1) / 2.0;
    
    // Test 1: Linear
    for(int i = 0; i < m; i++) sendbuf[i] = rank + 1;
    linear_allreduce(sendbuf, recvbuf, m, MPI_COMM_WORLD);
    printf("Rank %d | Linear              | %.1f | %s\n", 
           rank, recvbuf[0], (recvbuf[0] == expected) ? "PASS" : "FAIL");
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Test 2: Rabenseifner
    for(int i = 0; i < m; i++) sendbuf[i] = rank + 1;
    rabenseifner_allreduce(sendbuf, recvbuf, m, MPI_COMM_WORLD);
    printf("Rank %d | Rabenseifner        | %.1f | %s\n", 
           rank, recvbuf[0], (recvbuf[0] == expected) ? "PASS" : "FAIL");
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Test 3: Ring
    for(int i = 0; i < m; i++) sendbuf[i] = rank + 1;
    ring_allreduce(sendbuf, recvbuf, m, MPI_COMM_WORLD);
    printf("Rank %d | Ring                | %.1f | %s\n", 
           rank, recvbuf[0], (recvbuf[0] == expected) ? "PASS" : "FAIL");
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Test 4: Ring Segmented
    for(int i = 0; i < m; i++) sendbuf[i] = rank + 1;
    ring_seg_allreduce(sendbuf, recvbuf, m, MPI_COMM_WORLD);
    printf("Rank %d | Ring Segmented      | %.1f | %s\n", 
           rank, recvbuf[0], (recvbuf[0] == expected) ? "PASS" : "FAIL");
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Test 5: Recursive Doubling
    for(int i = 0; i < m; i++) sendbuf[i] = rank + 1;
    recursive_doubling_allreduce(sendbuf, recvbuf, m, MPI_COMM_WORLD);
    printf("Rank %d | Recursive Doubling  | %.1f | %s\n", 
           rank, recvbuf[0], (recvbuf[0] == expected) ? "PASS" : "FAIL");
    
    free(sendbuf);
    free(recvbuf);
    MPI_Finalize();
    return 0;
}