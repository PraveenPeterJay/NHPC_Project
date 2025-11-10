#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h> 
#include <string.h> // For memcpy

// --- WARMUP AND MEASUREMENT CONFIGURATION ---
#define N_WARMUP 1
#define N_ITERATIONS 1
#define MICRO_PAUSE_TICKS 0 
// ------------------------------------------

// --- NEW CONFIGURATION CONSTANTS ---
#define NUM_TESTS 12           // Number of message sizes to test (at least ten)
#define START_MESSAGE_SIZE 2048 // Start size in doubles
#define SIZE_INCREMENT 512     // Increment size in doubles
// -----------------------------------

/**
 * @brief Implements the Linear Chain Allreduce using MPI_Send and MPI_Recv.
 * Steps: 1. Reduce to Root (Rank 0) via P2P chain. 2. Broadcast from Root via P2P chain.
 */
void custom_linear_allreduce_p2p(double *send_buf, double *recv_buf, int count, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    const int ROOT = 0;
    
    double *temp_buf = (double*)malloc(count * sizeof(double));
    if (!temp_buf) {
        // In a real application, proper error handling is needed. Here, we exit.
        MPI_Abort(comm, 99); 
    }

    // Initialize recv_buf with local data, as it will hold the accumulating sum
    memcpy(recv_buf, send_buf, count * sizeof(double));

    // --- PHASE 1: REDUCE TO ROOT (P-1 steps) ---
    // Rank i receives from (i+1) and sends to (i-1), where applicable.
    for (int i = size - 1; i > ROOT; i--) {
        if (rank == i) {
            // My rank is 'i'. I send my current partial sum (in recv_buf) to my left neighbor (i-1)
            // Use tag 0 for Reduce phase
            MPI_Send(recv_buf, count, MPI_DOUBLE, rank - 1, 0, comm); 
        } else if (rank == i - 1) {
            // My rank is 'i-1'. I receive data from my right neighbor (i)
            // Use tag 0 for Reduce phase
            MPI_Recv(temp_buf, count, MPI_DOUBLE, rank + 1, 0, comm, MPI_STATUS_IGNORE);
            
            // Perform local reduction (summation)
            for (int j = 0; j < count; j++) {
                recv_buf[j] += temp_buf[j];
            }
        }
    }
    
    // CRITICAL BARRIER: Forces simulation to complete Reduce phase before starting Bcast.
    MPI_Barrier(comm); 

    // --- PHASE 2: BROADCAST FROM ROOT (P-1 steps) ---
    // Rank i sends the final result (in recv_buf) to (i+1)
    for (int i = 0; i < size - 1; i++) {
        if (rank == i) {
            // My rank is 'i'. I send the final result to my right neighbor (i+1)
            // Use tag 1 for Broadcast phase
            MPI_Send(recv_buf, count, MPI_DOUBLE, rank + 1, 1, comm);
        } else if (rank == i + 1) {
            // My rank is 'i+1'. I receive the result from my left neighbor (i)
            // Use tag 1 for Broadcast phase
            MPI_Recv(recv_buf, count, MPI_DOUBLE, rank - 1, 1, comm, MPI_STATUS_IGNORE);
            
            // Note: No computation (gamma) is done here, just data distribution.
        }
    }

    free(temp_buf);
}

// Simple busy-wait function for the micro-pause (Now effectively a NO-OP with ticks=0)
void busy_wait(long ticks) {
    if (ticks > 0) {
        clock_t start = clock();
        while ((clock() - start) < ticks);
    }
}


int main(int argc, char *argv[]) {
    int rank, size;
    
    // Initialize MPI Environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    const int ROOT = 0;
    const int P = size;

    if (P < 2) {
        if (rank == 0) fprintf(stderr, "\033[91mError: Requires at least 2 processes.\033[0m\n");
        MPI_Finalize();
        return 1;
    }

    // Initialize random seed (used for data perturbation)
    srand(rank + time(NULL));

    // Rank 0 prints the CSV header
    if (rank == ROOT) {
        // P: Process Count, m: Message Size (doubles), T: Time (sec)
        // X1, X2, X3: Regression independent variables
        printf("P,m,T,X1,X2,X3\n");
    }
    
    // --- Dynamically calculate message sizes (m) based on additive increase ---
    int MESSAGE_SIZES[NUM_TESTS];
    for (int i = 0; i < NUM_TESTS; i++) {
        // m starts at 2048 and increases by 1024 for each test
        MESSAGE_SIZES[i] = START_MESSAGE_SIZE + i * SIZE_INCREMENT;
    }
    // Resulting sizes will be: 2048, 3072, 4096, 5120, 6144, 7168, 8192, 9216, 10240, 11264, 12288
    // -------------------------------------------------------------------------
    
    // Test for each defined message size
    for (int i = 0; i < NUM_TESTS; i++) {
        int m = MESSAGE_SIZES[i];
        
        // --- Setup Data (m doubles) ---
        double *send_buf = (double*)calloc(m, sizeof(double)); 
        double *recv_buf = (double*)calloc(m, sizeof(double)); 
        
        if (!send_buf || !recv_buf) {
            fprintf(stderr, "\033[91mError: Memory allocation failed for size %d on rank %d.\033[0m\n", m, rank);
            if (send_buf) free(send_buf);
            if (recv_buf) free(recv_buf);
            MPI_Finalize();
            return 1;
        }

        // --- PHASE 1: WARMUP ---
        for (int iter = 0; iter < N_WARMUP; iter++) {
            // Data perturbation
            for (int j = 0; j < m; j++) {
                send_buf[j] = 1.0 + (double)(rand() % 100) * 1e-12;
            }
            
            // Reinitialize recv_buf with local data for the P2P custom routine
            memcpy(recv_buf, send_buf, m * sizeof(double)); 

            MPI_Barrier(MPI_COMM_WORLD);
            custom_linear_allreduce_p2p(send_buf, recv_buf, m, MPI_COMM_WORLD);
            
            busy_wait(MICRO_PAUSE_TICKS);
        }


        // --- PHASE 2: MEASUREMENT ---
        double total_time = 0.0;
        for (int iter = 0; iter < N_ITERATIONS; iter++) {
            // Data perturbation
            for (int j = 0; j < m; j++) {
                send_buf[j] = 1.0 + (double)(rand() % 100) * 1e-12;
            }
            // Reinitialize recv_buf with local data for the P2P custom routine
            memcpy(recv_buf, send_buf, m * sizeof(double)); 

            MPI_Barrier(MPI_COMM_WORLD);
            double start_time = MPI_Wtime();
            
            // Execute the custom Linear Allreduce
            custom_linear_allreduce_p2p(send_buf, recv_buf, m, MPI_COMM_WORLD);
            
            MPI_Barrier(MPI_COMM_WORLD);
            double end_time = MPI_Wtime();
            
            total_time += (end_time - start_time);
            
            busy_wait(MICRO_PAUSE_TICKS);
        }

        double avg_time = total_time / N_ITERATIONS;

        // --- Regression Variable Calculation (on Root) ---
        if (rank == ROOT) {
            // T(P, m) = X1*alpha + X2*beta + X3*gamma
            double X1 = 2.0 * (P - 1);
            double X2 = 2.0 * (P - 1) * m;
            double X3 = 1.0 * (P - 1) * m;
            
            // Print data point
            printf("%d,%d,%.9f,%.0f,%.0f,%.0f\n", P, m, avg_time, X1, X2, X3);
        }

        // Clean up memory
        free(send_buf);
        free(recv_buf);
    }

    MPI_Finalize();
    return 0;
}