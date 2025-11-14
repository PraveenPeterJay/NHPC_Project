#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <string.h>

// --- WARMUP AND MEASUREMENT CONFIGURATION ---
#define N_WARMUP 0
#define N_ITERATIONS 1
#define MICRO_PAUSE_TICKS 0
// ------------------------------------------

// --- CONFIGURATION CONSTANTS ---
#define NUM_TESTS 100
#define START_MESSAGE_SIZE 8192  // Start size in doubles
#define SIZE_INCREMENT 128       // Increment size in doubles
// -----------------------------------

/**
 * @brief Implements Ring Allreduce without segmentation (Rabenseifner-style)
 * 
 * This is a simplified, correct implementation based on the standard ring algorithm.
 * 
 * Algorithm:
 * Phase 1 (Reduce-Scatter): P-1 steps
 *   - Each process starts by sending chunk i to its right neighbor
 *   - In each step, receive from left, reduce, then send to right
 *   - After P-1 steps, each process has one fully reduced chunk
 * 
 * Phase 2 (Allgather): P-1 steps
 *   - Send the reduced chunk around the ring
 *   - No reduction, just forwarding
 */
void custom_ring_allreduce_no_segmentation(double *send_buf, double *recv_buf, 
                                           int count, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    const int P = size;
    
    // Calculate chunk size
    int chunk_size = count / P;
    int remainder = count % P;
    
    // Allocate temporary buffer
    double *temp_buf = (double*)malloc(count * sizeof(double));
    if (!temp_buf) {
        MPI_Abort(comm, 99);
    }
    
    // Initialize recv_buf with local data
    memcpy(recv_buf, send_buf, count * sizeof(double));
    
    // Ring neighbors
    int left = (rank - 1 + P) % P;
    int right = (rank + 1) % P;
    
    // --- PHASE 1: REDUCE-SCATTER ---
    // After this phase, rank r will have the fully reduced chunk r
    
    int recv_chunk = (rank - 1 + P) % P;  // Start by receiving the chunk to our left
    
    for (int step = 0; step < P - 1; step++) {
        int send_chunk = recv_chunk;
        recv_chunk = (recv_chunk - 1 + P) % P;
        
        // Calculate offsets and counts
        int send_offset = send_chunk * chunk_size;
        int send_count = (send_chunk == P - 1) ? chunk_size + remainder : chunk_size;
        
        int recv_offset = recv_chunk * chunk_size;
        int recv_count = (recv_chunk == P - 1) ? chunk_size + remainder : chunk_size;
        
        // Exchange data
        MPI_Sendrecv(
            &recv_buf[send_offset], send_count, MPI_DOUBLE, right, 0,
            &temp_buf[recv_offset], recv_count, MPI_DOUBLE, left, 0,
            comm, MPI_STATUS_IGNORE
        );
        
        // Reduce into recv_buf
        for (int i = 0; i < recv_count; i++) {
            recv_buf[recv_offset + i] += temp_buf[recv_offset + i];
        }
    }
    
    MPI_Barrier(comm);
    
    // --- PHASE 2: ALLGATHER ---
    // Distribute the fully reduced chunks
    
    int send_chunk = rank;  // Start by sending our own fully reduced chunk
    
    for (int step = 0; step < P - 1; step++) {
        recv_chunk = (send_chunk - 1 + P) % P;
        
        // Calculate offsets and counts
        int send_offset = send_chunk * chunk_size;
        int send_count = (send_chunk == P - 1) ? chunk_size + remainder : chunk_size;
        
        int recv_offset = recv_chunk * chunk_size;
        int recv_count = (recv_chunk == P - 1) ? chunk_size + remainder : chunk_size;
        
        // Exchange data (no reduction)
        MPI_Sendrecv(
            &recv_buf[send_offset], send_count, MPI_DOUBLE, right, 1,
            &recv_buf[recv_offset], recv_count, MPI_DOUBLE, left, 1,
            comm, MPI_STATUS_IGNORE
        );
        
        send_chunk = recv_chunk;
    }
    
    free(temp_buf);
}

// Simple busy-wait function (NO-OP with ticks=0)
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
        if (rank == 0) {
            fprintf(stderr, "\033[91mError: Requires at least 2 processes.\033[0m\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Initialize random seed
    srand(rank + time(NULL));

    // Rank 0 prints the CSV header
    if (rank == ROOT) {
        printf("P,m,T,X1,X2,X3\n");
    }
    
    // Dynamically calculate message sizes
    int MESSAGE_SIZES[NUM_TESTS];
    for (int i = 0; i < NUM_TESTS; i++) {
        MESSAGE_SIZES[i] = START_MESSAGE_SIZE + i * SIZE_INCREMENT;
    }
    
    // Test for each defined message size
    for (int i = 0; i < NUM_TESTS; i++) {
        int m = MESSAGE_SIZES[i];
        
        // Setup Data (m doubles)
        double *send_buf = (double*)calloc(m, sizeof(double)); 
        double *recv_buf = (double*)calloc(m, sizeof(double)); 
        
        if (!send_buf || !recv_buf) {
            fprintf(stderr, "\033[91mError: Memory allocation failed for size %d on rank %d.\033[0m\n", 
                    m, rank);
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
            
            memcpy(recv_buf, send_buf, m * sizeof(double));

            MPI_Barrier(MPI_COMM_WORLD);
            custom_ring_allreduce_no_segmentation(send_buf, recv_buf, m, MPI_COMM_WORLD);
            
            busy_wait(MICRO_PAUSE_TICKS);
        }

        // --- PHASE 2: MEASUREMENT ---
        double total_time = 0.0;
        for (int iter = 0; iter < N_ITERATIONS; iter++) {
            // Data perturbation
            for (int j = 0; j < m; j++) {
                send_buf[j] = 1.0 + (double)(rand() % 100) * 1e-12;
            }
            
            memcpy(recv_buf, send_buf, m * sizeof(double));

            MPI_Barrier(MPI_COMM_WORLD);
            double start_time = MPI_Wtime();
            
            // Execute the custom Ring Allreduce
            custom_ring_allreduce_no_segmentation(send_buf, recv_buf, m, MPI_COMM_WORLD);
            
            MPI_Barrier(MPI_COMM_WORLD);
            double end_time = MPI_Wtime();
            
            total_time += (end_time - start_time);
        }

        double avg_time = total_time / N_ITERATIONS;

        // --- Regression Variable Calculation (on Root) ---
        if (rank == ROOT) {
            // From SUARA paper Table 1, Ring without segmentation:
            // T(P,m) = 2*(P-1)*alpha + 2*((P-1)/P)*beta*m + ((P-1)/P)*gamma*m
            double X1 = 2.0 * (P - 1);              // Latency coefficient
            double X2 = 2.0 * ((P - 1.0) / P) * m;  // Bandwidth coefficient
            double X3 = ((P - 1.0) / P) * m;        // Computation coefficient
            
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