#include"../macros.h"




//typedef double (*getTimeandPc)(ll, ll, double, double, double, ll*);
typedef long long ll;

double lin_rd(long long P, long long m, long long ms, double alpha_beta_gamma[3][NUM_ALGOS+1], ll * Pc_ptr){
	long long pow1 = 1;

	double alpha_row = alpha_beta_gamma[0][LINEAR_ALL_REDUCE];
	double beta_row = alpha_beta_gamma[1][LINEAR_ALL_REDUCE];
	double gamma_row = alpha_beta_gamma[2][LINEAR_ALL_REDUCE];
	double alpha_column = alpha_beta_gamma[0][RECURSIVE_DOUBLING_ALL_REDUCE];
	double beta_column = alpha_beta_gamma[1][RECURSIVE_DOUBLING_ALL_REDUCE];
	double gamma_column =  alpha_beta_gamma[2][RECURSIVE_DOUBLING_ALL_REDUCE];

	while(pow1 <= P){
		pow1 *= 2;
	}	
	pow1 /= 2;
	//now pow1 is the highest power of 2 in P
	ll Pc_opt=1;
	double t_opt=1000;
	if(P == pow1){
		//brute force check all pow1 combinations
		for(int Pc_cand=1; Pc_cand < P; Pc_cand *= 2){
			double t_cand = hockneytime_lin(Pc_cand, m, ms, alpha_row, beta_row, gamma_row)
							+ hockneytime_rab(P/Pc_cand, m, ms, alpha_column, beta_column, gamma_column);
			if(t_cand < t_opt){
				Pc_opt = Pc_cand;
				t_opt = t_cand;
			}
		}
		
	}
	else{
		//use the derivative
		return 1;

	}
	Pc_ptr = &Pc_opt;
	return t_opt;
}
