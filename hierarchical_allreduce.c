#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

/**
 * @brief Performs a two-step hierarchical Allreduce on a generalized grid (R x X).
 * The number of total processes (P) and columns (Pc) are read dynamically.
 * * Usage:
 * 	smpicc hierarchical_allreduce.c 
 * 	smpirun -n <P> -platform platform.xml ./a.out <Pc>
 */
int main(int argc, char *argv[]) {
    int rank, size;
    int cols, rows; // X, R
    int data_vector_size; // N (Total Processes)
    int row_id, col_id;
    int key;

    // Initialize MPI Environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // --- 1. Read and Validate Command Line Argument ---
    if (argc < 2) {
        if (rank == 0) {
            fprintf(stderr, "\033[91mUsage: mpirun -np <P> %s <Pc>\n\033[0m", argv[0]);
            fprintf(stderr, "  <P>: Total number of processes (must match -np P)\n");
            fprintf(stderr, "  <Pc>: Number of columns (max row width)\n");
        }
        MPI_Finalize();
        return 1;
    }

    cols = atoi(argv[1]);
    data_vector_size = size; // N

    if (cols <= 0 || cols > size) {
        if (rank == 0) {
            fprintf(stderr, "\033[91mError: Number of columns (%d) must be between 1 and P (%d).\n\033[0m", cols, size);
        }
        MPI_Finalize();
        return 1;
    }

    if (size%cols != 0) {
        if (rank == 0) {
            fprintf(stderr, "\033[91mError: Number of columns (%d) must be a factor of P (%d).\n\033[0m", cols, size);
        }
        MPI_Finalize();
        return 1;
    }
    
    // Calculate Rows (R)
    rows = size / cols;

    // --- 2. Setup Initial Data (Dynamic Allocation) ---
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

    // --- 3. Define the Grid and Subgroups ---

    // Define Row and Column indices based on COLS
    row_id = rank / cols; 
    col_id = rank % cols;

    // Initialize the vector: Pi has (i*N+1) to (i*N+N)
    // printf("Process %d (Row %d, Column: %d):- Initial vector: [", rank, row_id, col_id);
    for (int i = 0; i < data_vector_size; i++) {
         initial_data[i] = (double)(rank * data_vector_size + i + 1);
         local_sum[i] = initial_data[i];
    //     printf("%.0f%s", initial_data[i], (i == data_vector_size - 1) ? "" : ", ");
     }
    // printf("]\n");

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();


    // --- STEP 1: Allreduce Across Rows (Processes with the same row_id) ---
    
    // Create Row Communicator (Color = row_id)
    MPI_Comm row_comm;
    key = rank;
    MPI_Comm_split(MPI_COMM_WORLD, row_id, key, &row_comm);
    
    // Perform Allreduce within each row on the full vector
    MPI_Allreduce(local_sum, row_result, data_vector_size, MPI_DOUBLE, MPI_SUM, row_comm);
    
    // Update local_sum with the result of the row Allreduce
    for (int i = 0; i < data_vector_size; i++) {
        local_sum[i] = row_result[i];
    }
    
    // --- Print Full Array After Row Allreduce ---
    // printf("Process %d (Row %d, Column: %d):- After ROW Allreduce: [", rank, row_id, col_id);
    // for (int i = 0; i < data_vector_size; i++) {
    //     printf("%.0f%s", local_sum[i], (i == data_vector_size - 1) ? "" : ", ");
    // }
    // printf("]\n");

    // --- STEP 2: Allreduce Across Columns (Processes with the same col_id) ---

    // Create Column Communicator (Color = col_id)
    MPI_Comm col_comm;
    key = rank; 
    MPI_Comm_split(MPI_COMM_WORLD, col_id, key, &col_comm);

    // Perform Allreduce within each column, using the row result (local_sum) as input
    MPI_Allreduce(local_sum, col_result, data_vector_size, MPI_DOUBLE, MPI_SUM, col_comm);

    // Update final local_sum
    for (int i = 0; i < data_vector_size; i++) {
        local_sum[i] = col_result[i];
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    double total_time = end_time - start_time;
    double message_size_bytes = data_vector_size * sizeof(double);
    double total_data_exchanged = 2 * message_size_bytes * (log2(size));
    double effective_bandwidth = total_data_exchanged / total_time;

    // --- 4. Final Verification ---
    
    // printf("\033[92mProcess %d: Final vector: [", rank);
    // for (int i = 0; i < data_vector_size; i++) {
    //     printf("%.0f%s", local_sum[i], (i == data_vector_size - 1) ? "" : ", ");
    // }
    // printf("]\033[0m\n");

    // Clean up
    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);
    free(initial_data); free(local_sum); free(row_result); free(col_result); 

    if (rank == 0) {
        printf("\n=== Simulation Summary ===\n");
        printf("Processes: %d\n", size);
        printf("Data vector size: %d doubles (%.2f KB)\n", data_vector_size, message_size_bytes / 1024.0);
        printf("Total time: %.6f sec\n", total_time);
        printf("Effective bandwidth (approx.): %.3f MB/s\n", effective_bandwidth / (1024 * 1024));
        printf("Configured latency: 50 us | Configured link BW: 125 MB/s\n");
        printf("===========================\n");
    }

    MPI_Finalize();
    return 0;
}
