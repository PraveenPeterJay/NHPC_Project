#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include"est_time.h"

/**
 * @brief Performs a two-step hierarchical Allreduce on a generalized grid (R x C).
 * The number of total processes (P) and columns (Pc) are read dynamically.
 * * Usage:
 * 	smpicc hierarchical_allreduce.c 
 * 	smpirun -n <P> -platform platform.xml ./a.out <Pc>
 */
int main(int argc, char *argv[]) {
    int rank, size;
    int data_vector_size; // N (Total Processes)
    ll row_id, col_id;
    ll key;

    // Initialize MPI Environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    data_vector_size = size; // N
    
    ll P = size;
    ll m = atoi(argv[1]);
    ll ms = m/P;
    ll * ans = malloc(3*sizeof(int));

    // Allocate arrays for input and intermediate results
    double *initial_data = (double*)malloc(data_vector_size * sizeof(double));
    double *local_sum = (double*)malloc(data_vector_size * sizeof(double)); 
    double *row_result = (double*)malloc(data_vector_size * sizeof(double));
    double *col_result = (double*)malloc(data_vector_size * sizeof(double));
    
    if (!initial_data || !local_sum || !row_result || !col_result) {
        if (rank == 0) fprintf(stderr, "\033[91mError: Memory allocation failed.\033[0m\n");
        free(initial_data); free(local_sum); free(row_result); free(col_result); 
        MPI_Finalize();
        return 1;
    }

    // Initialize the vector: Pi has (i*N+1) to (i*N+N)
    // printf("Process %d (Row %d, Column: %d):- Initial vector: [", rank, row_id, col_id);
    for (int i = 0; i < data_vector_size; i++) {
         initial_data[i] = (double)(rank * data_vector_size + i + 1);
         local_sum[i] = initial_data[i];
    //     printf("%.0f%s", initial_data[i], (i == data_vector_size - 1) ? "" : ", ");
     }
    // printf("]\n");

    double start_time = MPI_Wtime();

    ll rows, cols;

    cols = P;
    rows = size / cols;
    row_id = rank / cols; 
    col_id = rank % cols;

    MPI_Barrier(MPI_COMM_WORLD);
    double mid_time = MPI_Wtime();

    
    // Perform Allreduce within each row on the full vector
    MPI_Allreduce(local_sum, row_result, data_vector_size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    
    // Update local_sum with the result of the row Allreduce
    for (int i = 0; i < data_vector_size; i++) {
        local_sum[i] = row_result[i];
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    double stage1_time = mid_time - start_time;
    double allreduce_time = end_time - mid_time;
    double total_time = end_time - start_time;
    double message_size_bytes = data_vector_size * sizeof(double);

    // Clean up
    free(initial_data); free(local_sum); free(row_result); free(col_result); 

    if (rank == 0) {
        printf("\n=== Simulation Summary ===\n");
        printf("Processes: %d\n", size);
        printf("Data vector size: %d doubles (%.2f KB)\n", data_vector_size, message_size_bytes / 1024.0);
        printf("Stage 1 time: %.6f sec\n", stage1_time);
        printf("All reduce time: %.6f sec\n", allreduce_time);
        printf("Total time: %.6f sec\n", total_time);
        // printf("Effective bandwidth (approx.): %.3f MB/s\n", effective_bandwidth / (1024 * 1024));
        // printf("Configured latency: 50 us | Configured link BW: 125 MB/s\n");
        printf("===========================\n");
    }

    MPI_Finalize();
    return 0;
}
