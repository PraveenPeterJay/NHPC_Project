#include<stdio.h>
#include<stdlib.h>
#include"macros.h"
#include"./utils/all_combos.h"
#include<string.h>
//#define LINEAR_ALL_REDUCE 0
//#define RABENSEIFNER_ALL_REDUCE 1
//#define RING_ALL_REDUCE 2
//#define RING_SEG_ALL_REDUCE 3
//#define RECURSIVE_DOUBLING_ALL_REDUCE 4


#define MAX_FIELD 256


getTimeandPc func[NUM_ALGOS+1][NUM_ALGOS+1];
double alpha_beta_gamma[3][NUM_ALGOS+1];


void my_init(char path[]){
	//read alpha beta and gamma from a csv
	FILE*fp = fopen(path, "r");
	char line[256];
    int i = 0;

    // Skip header line
    fgets(line, sizeof(line), fp);
	printf("-------------------------------------------------------------\n");
	printf("%-30s | %-20s %-20s %-20s\n", "Algorithm", "Alpha", "Beta", "Gamma");
	printf("-------------------------------------------------------------\n");

	while (fgets(line, sizeof(line), fp) && i < NUM_ALGOS) {
		char algo[MAX_FIELD];
		double a, b, c;

		if (sscanf(line, "%[^,],%lf,%lf,%lf", algo, &a, &b, &c) == 4) {
			alpha_beta_gamma[0][i] = a;
			alpha_beta_gamma[1][i] = b;
			alpha_beta_gamma[2][i] = c;

			// Pretty formatted row
			printf("%-15s | %-10.4f %-10.4f %-10.4f\n", algo, a, b, c);
			i++;
		}
	}

	printf("-------------------------------------------------------------\n");


    fclose(fp);


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

	printf("Init Done!\n");
}


double Stage1(ll P, ll m, ll ms, double alpha_beta_gamma[][NUM_ALGOS+1], ll * ans){
	//recall that Pc is the number of columns
	//within a row ar is the algorithm used.
	printf("Stage1 called\n");
	double min_time = 1e5;
	ll Pc_opt = P;
	int algorow_opt, algocol_opt;
	
	if(ans == NULL)
		ans = malloc(3*sizeof(ll));
	//assume that ans already has space
	//that is just a fallback
	
	
	for(int i=0; i<NUM_ALGOS; i++){
		for(int j=0; j<NUM_ALGOS; j++){
			ll * pc_cand = malloc(1*sizeof(ll));
			// printf("going to call a function %d %d\n", i, j);
			double time_taken = func[i][j](P, m, ms, alpha_beta_gamma,pc_cand);
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

void printtimes(){
	
}

int main(){
	my_init("./data_store/sample.csv");
	ll P = 1024, m = 1024;
	ll ms = m/P;
	ll * ans = malloc(3*sizeof(ll));
	Stage1(P, m, ms, alpha_beta_gamma, ans);
	printtimes();


	for(int i=0; i<3; i++){
		printf("ans[%d] is %d\n", i, ans[i]);
	}
	return 0;
}
 
