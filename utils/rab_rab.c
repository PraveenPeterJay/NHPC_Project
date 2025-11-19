#include"../macros.h"




//typedef double (*getTimeandPc)(ll, ll, double, double, double, ll*);
typedef long long ll;

double rab_rab(long long P, long long m, long long ms, double alpha_beta_gamma[3][NUM_ALGOS+1], ll * Pc_ptr){
	long long pow1 = 1;

	double alpha_row = alpha_beta_gamma[0][RABENSEIFNER_ALL_REDUCE];
	double beta_row = alpha_beta_gamma[1][RABENSEIFNER_ALL_REDUCE];
	double gamma_row = alpha_beta_gamma[2][RABENSEIFNER_ALL_REDUCE];
	double alpha_column = alpha_beta_gamma[0][RABENSEIFNER_ALL_REDUCE];
	double beta_column = alpha_beta_gamma[1][RABENSEIFNER_ALL_REDUCE];
	double gamma_column =  alpha_beta_gamma[2][RABENSEIFNER_ALL_REDUCE];

	while(pow1 <= P){
		pow1 *= 2;
	}	
	pow1 /= 2;
	//now pow1 is the highest power of 2 in P
	ll Pc_opt=1;
	int hockney_time_min=1000;
	if(P == pow1){
		//brute force check all pow1 combinations
		
	}
	else{
		//use the derivative

	}
	Pc_ptr = &Pc_opt;
	return hockney_time_min;
}
