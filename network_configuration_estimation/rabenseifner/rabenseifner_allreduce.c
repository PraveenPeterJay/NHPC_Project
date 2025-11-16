//DEBUG2:
//DEBUG2:


#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h> 
#include <string.h> // For memcpy
#include "rabenseifner_allreduce.h" //For unit-testing rabenseifner function

// --- WARMUP AND MEASUREMENT CONFIGURATION ---
#define N_WARMUP 0
#define N_ITERATIONS 1
#define MICRO_PAUSE_TICKS 0 
// ------------------------------------------

// --- NEW CONFIGURATION CONSTANTS ---
#define NUM_TESTS 100           
#define START_MESSAGE_SIZE 8192 // Start size in doubles
#define SIZE_INCREMENT 128     // Increment size in doubles
// -----------------------------------

/**
 * @brief  Implements the Rabenseifner Allreduce using MPI_Send and MPI_Recv.
 * Steps: 1. Reduce-Scatter via recursive halving. 2. Allgather via recursive doubling.
 * Assumes P is a power of 2 for simplicity.
**/
void custom_rabenseifner_allreduce_p2p(double *send_buf, double *recv_buf, int count, MPI_Comm comm) {

   int rank, size;
   MPI_Comm_rank(comm, &rank);
   MPI_Comm_size(comm, &size);

    //Error handling:
    //a. if not power of 2, print error 
    if ((size & (size-1)) != 0){
        if (rank == 0){
            fprintf(stderr, "For Rabenseifner, you should have power of 2 processes");
        }
        MPI_Abort(comm, 1);
    }

    //DEBUG1:
    // // --- ADD: open per-rank log file ---
    // char logname[64];
    // sprintf(logname, "trace_rank_%d.log", rank);
    // FILE *trace = fopen(logname, "w");
    //DEBUG1:


    memcpy(recv_buf, send_buf, count * sizeof(double));
    double *tempbuf = (double *) malloc(count * sizeof(double));             //tempbuf is needed bcoz we want to store the received elements, and then add them onto recvbuf

    int recv_size = count;
    int recv_offset = 0;
    int send_offset = 0;
    int mask = 1;

    //DEBUG2:
    // int reduce_step = 0;   // logging counters
    // int gather_step = 0;
    //DEBUG2:

    //PHASE1: Reduce Scatter
    while(mask < size){          //OR (phase < no_of_phases)     //NOTE: mask = pow(2, phase)
        int partner = rank ^ mask;

        recv_size /= 2;         //Example values for 8 nodes: {4, 2, 1}



        if (rank < partner){        //am on left half of partner, therefore receive left_half
            //receive leftHalf (send right half)
            send_offset = recv_offset + recv_size;      //& No change in recv_offset

            //DEBUG3:
            // fprintf(trace,
            //     "[ReduceScatter step %d] Rank %d  partner %d\n"
            //     "   SEND   offset=%d  size=%d\n"
            //     "   RECV   offset=%d  size=%d\n",
            //     reduce_step, rank, partner,
            //     send_offset, recv_size,
            //     recv_offset, recv_size);
            // fflush(trace);
            //DEBUG3:

            MPI_Request send_req, recv_req;
            MPI_Irecv(tempbuf, recv_size, MPI_DOUBLE, partner, 0, comm, &recv_req);
            MPI_Isend(&recv_buf[send_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &send_req);

            MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
            MPI_Wait(&send_req, MPI_STATUS_IGNORE);

            //Add up the data stored in tempbuf to recvbuf
            for (int i = 0; i < recv_size; i++) {
                recv_buf[recv_offset+i] += tempbuf[i];
            }

            //DEBUG:
            // fprintf(trace,
            // "Elements sent: \n");
            // for (int i = 0; i < recv_size; i++){
            //     fprintf(trace, "%f ",
            //     recv_buf[send_offset+i]);
            // }   fprintf(trace, "\n");
            // fprintf(trace,
            // "Elements received: \n");
            // for (int i = 0; i < recv_size; i++){
            //     fprintf(trace, "%f ",
            //     recv_buf[recv_offset+i]);
            // }   fprintf(trace, "\n\n\n");
            // fflush(trace);
            //DEBUG:
        }
        else{ //(rank > partner)      //am on right half of partner, therefore receive right half
            //receive rightHalf (send left half)
            send_offset = recv_offset;
            recv_offset += recv_size;       //Example values for node 1, total_nodes 8: {4, 4, 4}

            //DEBUG4:
            // fprintf(trace,
            //     "[ReduceScatter step %d] Rank %d  partner %d\n"
            //     "   SEND   offset=%d  size=%d\n"
            //     "   RECV   offset=%d  size=%d\n\n\n",
            //     reduce_step, rank, partner,
            //     send_offset, recv_size,
            //     recv_offset, recv_size);
            // fflush(trace);
            //DEBUG4:

            MPI_Request send_req, recv_req;
            MPI_Irecv(tempbuf, recv_size, MPI_DOUBLE, partner, 0, comm, &recv_req);
            MPI_Isend(&recv_buf[send_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &send_req);

            MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
            MPI_Wait(&send_req, MPI_STATUS_IGNORE);

            //Add up the data stored in tempbuf to recvbuf
            for (int i = 0; i < recv_size; i++){
                recv_buf[recv_offset+i] += tempbuf[i];
            }

            //DEBUG:
            // fprintf(trace,
            // "Elements sent: \n");
            // for (int i = 0; i < recv_size; i++){
            //     fprintf(trace, "%f ",
            //     recv_buf[send_offset+i]);
            // }   fprintf(trace, "\n");
            // fflush(trace);
            // fprintf(trace,
            // "Elements received: \n");
            // for (int i = 0; i < recv_size; i++){
            //     fprintf(trace, "%f ",
            //     recv_buf[recv_offset+i]);
            // }   fprintf(trace, "\n\n\n");
            // fflush(trace);
            //DEBUG:
        }

        mask *= 2;

        //DEBUG//
        // reduce_step++;
        //DEBUG//
    }

    send_offset = recv_offset;
    mask = size / 2;

   //PHASE2: ALLGATHER
   while(mask > 0){
        int partner = rank ^ mask;      

        if (rank < partner){        //I have correct left half, and partner has correct right half
            //send leftHalf (receive right half)
            send_offset = recv_offset;
            int partnerValue_offset = recv_offset + recv_size;      //& No change in send_offset

            //DEBUG5:
            // fprintf(trace,
            //     "[Allgather step %d] Rank %d  partner %d\n"
            //     "   SEND   offset=%d  size=%d\n"
            //     "   RECV   offset=%d  size=%d\n"
            //     "   (I am LOWER block)\n",                        //FIX THIS
            //     gather_step, rank, partner,
            //     send_offset, recv_size,
            //     partnerValue_offset, recv_size);
            // //DEBUG5:            

            MPI_Request send_req, recv_req;
            MPI_Irecv(&recv_buf[partnerValue_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &recv_req);
            MPI_Isend(&recv_buf[send_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &send_req);

            MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
            MPI_Wait(&send_req, MPI_STATUS_IGNORE);

            //DEBUG:
            // fprintf(trace,
            // "Elements sent: \n");
            // for (int i = 0; i < recv_size; i++){
            //     fprintf(trace, "%f ",
            //     recv_buf[send_offset+i]);
            // }   fprintf(trace, "\n");
            // fprintf(trace,
            // "Elements received: \n");
            // for (int i = 0; i < recv_size; i++){
            //     fprintf(trace, "%f ",
            //     recv_buf[recv_offset+i]);
            // }   fprintf(trace, "\n\n\n");
            // fflush(trace);
            //DEBUG:
        }
        else{   //if (rank > partner)   //I have correct right half, and partner has correct left half
            //send rightHalf (receive left half)
            send_offset = recv_offset;
            int partnerValue_offset = recv_offset - recv_size;      //& No change in send_offset

            //DEBUG6:
            // fprintf(trace,
            //     "[Allgather step %d] Rank %d  partner %d\n"
            //     "   SEND   offset=%d  size=%d\n"
            //     "   RECV   offset=%d  size=%d\n"
            //     "   (I am UPPER block)\n",
            //     gather_step, rank, partner,
            //     send_offset, recv_size,
            //     partnerValue_offset, recv_size);
            //DEBUG6:

            MPI_Request send_req, recv_req;
            MPI_Irecv(&recv_buf[partnerValue_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &recv_req);
            MPI_Isend(&recv_buf[send_offset], recv_size, MPI_DOUBLE, partner, 0, comm, &send_req);

            MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
            MPI_Wait(&send_req, MPI_STATUS_IGNORE);


            //DEBUG:
            // fprintf(trace,
            // "Elements sent: \n");
            // for (int i = 0; i < recv_size; i++){
            //     fprintf(trace, "%f ",
            //     recv_buf[send_offset+i]);
            // }   fprintf(trace, "\n");
            // fprintf(trace,
            // "Elements received: \n");
            // for (int i = 0; i < recv_size; i++){
            //     fprintf(trace, "%f ",
            //     recv_buf[recv_offset+i]);
            // }   fprintf(trace, "\n\n\n");
            // fflush(trace);
            //DEBUG:

            recv_offset = partnerValue_offset;
        }

        recv_size *= 2;
        mask /= 2;
        //DEBUG//
        // gather_step++;
        //DEBUG//
    }

    free(tempbuf);
    fclose(trace);
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

    // Check if P is a power of 2 (required for Rabenseifner)
    if ((P & (P - 1)) != 0) {
        if (rank == 0) {
            fprintf(stderr, "\033[91mError: Rabenseifner requires power-of-2 processes. Got %d.\033[0m\n", P);
            fprintf(stderr, "\033[93mPlease use P = 2, 4, 8, 16, 32, etc.\033[0m\n");
        }
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
            custom_rabenseifner_allreduce_p2p(send_buf, recv_buf, m, MPI_COMM_WORLD);
            
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
            custom_rabenseifner_allreduce_p2p(send_buf, recv_buf, m, MPI_COMM_WORLD);
            
            MPI_Barrier(MPI_COMM_WORLD);
            double end_time = MPI_Wtime();
            
            total_time += (end_time - start_time);
            
            // busy_wait(MICRO_PAUSE_TICKS);
        }

        double avg_time = total_time / N_ITERATIONS;

        // --- Regression Variable Calculation (on Root) ---
        if (rank == ROOT) {
            // Rabenseifner cost model: T(P, m) = X1*alpha + X2*beta + X3*gamma
            // log₂P steps in each phase (reduce-scatter and allgather)
            int log2P = 0;
            int temp_P = P;
            while (temp_P > 1) {
                log2P++;
                temp_P /= 2;
            }
            
            double X1 = 2.0 * log2P;
            double X2 = 2.0 * ((P - 1.0) / P) * m;
            double X3 = ((P - 1.0) / P) * m;
            
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