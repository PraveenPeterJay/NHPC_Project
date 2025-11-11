#include<stdio.h>
#include<stdlib.h>
#include"macros.h"
#include"utils/lin_rab.h"
#include"utils/lin_rd.h"
#include"utils/lin_rnos.h"
#include"utils/lin_rs.h"
#include"utils/rab_lin.h" 
#include"utils/rab_rd.h"
#include"utils/rab_rnos.h"
#include"utils/rab_rs.h"
#include"utils/rd_lin.h"
#include"utils/rd_rab.h"
#include"utils/rd_rnos.h"
#include"utils/rd_rs.h"
#include"utils/rnos_lin.h"
#include"utils/rnos_rab.h"
#include"utils/rnos_rd.h"
#include"utils/rnos_rs.h"
#include"utils/rs_lin.h"
#include"utils/rs_rab.h"
#include"utils/rs_rd.h"
#include"utils/rs_rnos.h"

//#define LINEAR_ALL_REDUCE 0
//#define RABENSEIFNER_ALL_REDUCE 1
//#define RING_ALL_REDUCE 2
//#define RING_SEG_ALL_REDUCE 3
//#define RECURSIVE_DOUBLING_ALL_REDUCE 4



double Stage1(ll P, ll m, ll ms, double alpha, double beta, double gamma, ll * ans){
	//recall that Pc is the number of columns
	//within a row ar is the algorithm used.
	
	double min_time = 1e5;
	ll Pc_opt = 0;
	int algorow_opt, algocol_opt;
	
	if(ans == NULL)
		ans = malloc(3*sizeof(ll));
	//assume that ans already has space
	//that is just a fallback
	
	getTimeandPc func[NUM_ALGOS][NUM_ALGOS];
        func[0][0] = lin_rab;
	func[0][1] = lin_rnos;
	func[0][2] = lin_rs;
	func[0][3] = lin_rd;	
	
	func[1][0] = rab_lin;
	func[1][1] = rab_rnos;
	func[1][2] = rab_rs;
	func[1][3] = rab_rd;	
	
	func[2][0] = rnos_lin;
	func[2][1] = rnos_rab;
	func[2][2] = rnos_rs;
	func[2][3] = rnos_rd;	
	
	func[3][0] = rs_lin;
	func[3][1] = rs_rab;
	func[3][2] = rs_rnos;
	func[3][3] = rs_rd;	
	
	func[4][0] = rd_lin;
	func[4][1] = rd_rab;
	func[4][2] = rd_rnos;
	func[4][3] = rd_rs;	

	for(int i=0; i<NUM_ALGOS; i++){
		for(int j=0; j<NUM_ALGOS; j++){
			ll * pc_cand = malloc(1*sizeof(ll));
			double time_taken = func[i][j](P, m, ms, alpha, beta, gamma, pc_cand);
			if(time_taken < min_time){
				algorow_opt = i;
				algocol_opt = j;
				Pc_opt = *pc_cand;
			}	
		}	
	}
	
	ans[0] = algorow_opt;
	ans[1] = algocol_opt;
	ans[2] = Pc_opt;
	return min_time;
}




 
