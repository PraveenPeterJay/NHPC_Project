//This code 
	//1. tries to estimate the time taken for different combination of allreduce algorithms
	//2. Then calculates the best algorithm after estimating the time, by using parameters alpha, beta, gamma, P (no. of processes) & m (message size)


#include<stdio.h>
#include<stdlib.h>
#include"macros.h"
#include"./utils/all_combos.h"			//Declares lin_lin, lin_rab, and similar 25 functions
#include<string.h>
#include "linear_allreduce.h"
#include "rabenseifner_allreduce.h"
#include "ring_allreduce.h"
#include "ring_seg_allreduce.h"
#include "recursive_doubling_allreduce.h"
//#define LINEAR_ALL_REDUCE 0
//#define RABENSEIFNER_ALL_REDUCE 1
//#define RING_ALL_REDUCE 2
//#define RING_SEG_ALL_REDUCE 3
//#define RECURSIVE_DOUBLING_ALL_REDUCE 4


#define MAX_FIELD 256


getTimeandPc func[NUM_ALGOS+1][NUM_ALGOS+1];					//func[i][j] estimates the time taken when using algorithm i for rows and j for columns
execAllReduce algo[NUM_ALGOS];									//algo[i] stores the function pointer that implements the algorithm defined by the macro i

double alpha_beta_gamma[3][NUM_ALGOS+1];						//alpha_beta_gamma[i][j] stores alpha, beta, gamma values for algorithm j


//Initialises our simulation by reading alpha, beta, gamma and loading function pointer tables
void my_init(char path[]){
    //1. read alpha beta and gamma from a csv
    FILE*fp = fopen(path, "r");
    char line[256];
    int i = 0;

    // Skip header line
    fgets(line, sizeof(line), fp);
	// printf("-------------------------------------------------------------\n");
	// printf("%-30s | %-20s %-20s %-20s\n", "Algorithm", "Alpha", "Beta", "Gamma");
	// printf("-------------------------------------------------------------\n");

	while (fgets(line, sizeof(line), fp) && i < NUM_ALGOS) {
		char algo[MAX_FIELD];
		double a, b, c;

		if (sscanf(line, "%[^,],%lf,%lf,%lf", algo, &a, &b, &c) == 4) {
			alpha_beta_gamma[0][i] = a;
			alpha_beta_gamma[1][i] = b;
			alpha_beta_gamma[2][i] = c;

			// Pretty formatted row
			// printf("%-15s | %-10.4f %-10.4f %-10.4f\n", algo, a, b, c);
			i++;
		}
	}

	// printf("-------------------------------------------------------------\n");


    fclose(fp);
	//2. Code for reading alpha, beta, gamma ends here

	//CHANGES NEEDED
	//assign all function pointers
	func[0][0] = lin_lin;
	func[0][1] = lin_rab;
	func[0][2] = lin_rnos;
	func[0][3] = lin_rs;
	func[0][4] = lin_rd;	

	func[1][0] = rab_lin;
	func[1][1] = rab_rab;
	func[1][2] = rab_rnos;
	func[1][3] = rab_rs;
	func[1][4] = rab_rd;	

	func[2][0] = rnos_lin;
	func[2][1] = rnos_rab;
	func[2][2] = rnos_rnos;
	func[2][3] = rnos_rs;
	func[2][4] = rnos_rd;

	func[3][0] = rs_lin;
	func[3][1] = rs_rab;
	func[3][2] = rs_rnos;
	func[3][3] = rs_rs;
	func[3][4] = rs_rd;

	func[4][0] = rd_lin;
	func[4][1] = rd_rab;
	func[4][2] = rd_rnos;
	func[4][3] = rd_rs;
	func[4][4] = rd_rd;

	//assign algo elements here
	algo[LINEAR_ALL_REDUCE] = linear_allreduce;
	algo[RABENSEIFNER_ALL_REDUCE] = rabenseifner_all_reduce;
	algo[RING_ALL_REDUCE] = ring_allreduce;
	algo[RING_SEG_ALL_REDUCE] = ring_seg_allreduce;
	algo[RECURSIVE_DOUBLING_ALL_REDUCE] = recursive_doubling_allreduce;

	// printf("Init Done!\n");
}

//Finds the optimal algorithm for a given set of parameters
double Stage1(ll P, ll m, ll ms, ll * ans){				//ans is a (1x3) array that stores 3 things: ans[0] stores optimal row algo, ans[1] stores optimal column algo, while ans[2] stores optimal Pc value
	//recall that Pc is the number of columns
	//within a row ar is the algorithm used.
	my_init("./data_store/sample.csv");
	// sleep(1);
	// printf("init done inside stage1\n");
	// printf("Stage1 called\n");
	double min_time = 1e10;
	find_and_store_factors(P);				//Finds the factors for P in factorsP[] array which is a global variable
	ll Pc_opt = P;
	int algorow_opt, algocol_opt;
	
	if(ans == NULL)
		ans = malloc(3*sizeof(ll));
	//assume that ans already has space
	//that is just a fallback
	
	
	for(int i=0; i<NUM_ALGOS; i++){
		for(int j=0; j<NUM_ALGOS; j++){
			ll * pc_cand = malloc(1*sizeof(ll));			//pc_cand stands for pc_candidate
			// printf("going to call a function %d %d\n", i, j);
			double time_taken_predicted = func[i][j](P, m, ms, alpha_beta_gamma,pc_cand);
			double time_taken_actual;



			//start time
			//initialisation of sendbuf, recvbuf, and other variables
			algo[i](const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
			//end_time
			//time_taken = end_time() - start_time()
			
			if(time_taken < min_time){
				algorow_opt = i;
				algocol_opt = j;
				Pc_opt = *pc_cand;
				min_time = time_taken;
			}	
		}	
	}
	
	ans[0] = algorow_opt;
	ans[1] = algocol_opt;
	ans[2] = Pc_opt;
	return min_time;
}


// %-15s | %-10.4f %-10.4f %-10.4f
void printtimes(){
	printf("TIMES ARE\n");
	printf("  | ");
	for(int j=0; j<NUM_ALGOS; j++){
		printf("%-10d ",  j);
	}
	printf("\n");
	printf("----------------------------------------------------------\n");
	for(int i=0; i<NUM_ALGOS; i++){
		printf("%d | ", i);
		for(int j=0; j<NUM_ALGOS; j++){
			printf("%-10.4lf ", times41[i][j]);
		}
		printf("\n");
	}
}

//Driver function
int main(){
	
	ll P = 1000, m = 1024;
	ll ms = m/P;
	ll * ans = malloc(3*sizeof(ll));
	Stage1(P, m, ms, ans);
	printtimes();


	for(int i=0; i<3; i++){
		printf("ans[%d] is %lld\n", i, ans[i]);
	}
	return 0;
}
 
