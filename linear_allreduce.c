#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

/**
 * @brief Performs a Linear Allreduce (MPI_Reduce to root, followed by MPI_Bcast from root)
 * and measures the time, printing data points for linear regression analysis.
 * * Usage: 
 * 1. Compile: smpicc linear_allreduce.c
 * 2. Run (e.g., P=4, P=8, P=16):
 * smpirun -np 4 ./a.out > data_P4.csv
 * smpirun -np 8 ./a.out > data_P8.csv
 * smpirun -np 16 ./a.out > data_P16.csv
 * 3. Combine all CSV files into a single master CSV.
 * 4. Run the Python analysis script: python3 analyze_regression.py
 */

// Define the array of message sizes (m) to test (in terms of doubles)
#define N_WARMUP 1
#define NUM_MESSAGE_SIZES 10
const int MESSAGE_SIZES[NUM_MESSAGE_SIZES] = {
    1, 10, 20, 100, 200, 500, 1000, 2000, 5000, 10000
};

// Number of times to run the collective for averaging
#define N_ITERATIONS 1

void linear_allreduce(double *send_buf, double *recv_buf, int count, MPI_Comm comm) {
    // Phase 1: Reduce to Root (Rank 0)
    MPI_Reduce(send_buf, recv_buf, count, MPI_DOUBLE, MPI_SUM, 0, comm);

    // Phase 2: Broadcast from Root (Rank 0)
    MPI_Bcast(recv_buf, count, MPI_DOUBLE, 0, comm);
}


int main(int argc, char *argv[]) {
    int rank, size;
    
    // Initialize MPI Environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Define Root and P
    const int ROOT = 0;
    const int P = size;

    if (P < 2) {
        if (rank == 0) fprintf(stderr, "\033[91mError: Requires at least 2 processes.\033[0m\n");
        MPI_Finalize();
        return 1;
    }

    // Rank 0 prints the CSV header
    if (rank == ROOT) {
        // P: Process Count, m: Message Size (doubles), T: Time (sec)
        // X1, X2, X3: Regression independent variables
        printf("P,m,T,X1,X2,X3\n");
    }
    
    // Test for each defined message size
    for (int i = 0; i < NUM_MESSAGE_SIZES; i++) {
        int m = MESSAGE_SIZES[i];
        
        // --- Setup Data (m doubles) ---
        double *send_buf = (double*)malloc(m * sizeof(double));
        double *recv_buf; // Will hold the result after Bcast
        
        if (rank == ROOT) {
            // Only the root needs a distinct receive buffer for MPI_Reduce
            recv_buf = (double*)malloc(m * sizeof(double));
        } else {
            // Non-root processes can reuse their send buffer for MPI_Bcast, 
            // but we use a dedicated recv_buf for cleaner logic (it's what MPI_Allreduce implies)
            recv_buf = (double*)malloc(m * sizeof(double));
        }
        
        if (!send_buf || !recv_buf) {
            fprintf(stderr, "\033[91mError: Memory allocation failed on rank %d.\033[0m\n", rank);
            if (send_buf) free(send_buf);
            if (recv_buf) free(recv_buf);
            MPI_Finalize();
            return 1;
        }

        // Initialize data: Each element is initialized to 1.0, so the final sum should be P.
        for (int j = 0; j < m; j++) {
            send_buf[j] = 1.0; 
        }

        for (int iter = 0; iter < N_WARMUP; iter++) {
            if (rank == ROOT) {
                // Reinitialize recv_buf on root before each reduce call
                for (int j = 0; j < m; j++) {
                    recv_buf[j] = 0.0; 
                }
            }
            // Barrier ensures all processes are synchronized before the collective starts
            MPI_Barrier(MPI_COMM_WORLD);
            linear_allreduce(send_buf, recv_buf, m, MPI_COMM_WORLD);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double total_time = 0.0;
        
        // --- Performance Measurement Loop ---
        for (int iter = 0; iter < N_ITERATIONS; iter++) {
            // Reinitialize recv_buf on root before each reduce call
            if (rank == ROOT) {
                for (int j = 0; j < m; j++) {
                    recv_buf[j] = 0.0; 
                }
            }

            MPI_Barrier(MPI_COMM_WORLD);
            double start_time = MPI_Wtime();
            
            // Execute the custom Linear Allreduce
            linear_allreduce(send_buf, recv_buf, m, MPI_COMM_WORLD);
            
            MPI_Barrier(MPI_COMM_WORLD);
            double end_time = MPI_Wtime();
            
            total_time += (end_time - start_time);

            // In the next iteration, everyone sends from the original data (send_buf),
            // which should remain unchanged for correctness.
        }

        double avg_time = total_time / N_ITERATIONS;

        // --- Regression Variable Calculation (on Root) ---
        if (rank == ROOT) {
            // T(P, m) = X1*alpha + X2*beta + X3*gamma
            double X1 = 2.0 * (P - 1);
            double X2 = 2.0 * (P - 1) * m;
            double X3 = 1.0 * (P - 1) * m;
            
            // Print data point: P, m, T, X1, X2, X3
            // The time is in seconds. The regression is done on the coefficients based on *word count* (m).
            printf("%d,%d,%.9f,%.0f,%.0f,%.0f\n", P, m, avg_time, X1, X2, X3);
            
            // Optional: Basic verification (all elements should be equal to P * 1.0 = P)
            // for (int j = 0; j < 1; j++) {
            //     if (fabs(recv_buf[j] - P) > 1e-6) {
            //         fprintf(stderr, "\033[91mWarning: Verification failed (P=%d, m=%d). Expected %.0f, got %.6f\033[0m\n", P, m, (double)P, recv_buf[j]);
            //     }
            // }
        }

        // Clean up memory
        free(send_buf);
        free(recv_buf);
    }

    MPI_Finalize();
    return 0;
}
