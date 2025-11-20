#include"../macros.h"
#include"hockneytime_rd.h"



//typedef double (*getTimeandPc)(ll, ll, double, double, double, ll*);


double rd_rd(long long P, long long m, long long ms, double alpha_beta_gamma[3][NUM_ALGOS+1], ll * Pc_ptr){
	long long pow1 = 1;

	double alpha_row = alpha_beta_gamma[0][RECURSIVE_DOUBLING_ALL_REDUCE];
	double beta_row = alpha_beta_gamma[1][RECURSIVE_DOUBLING_ALL_REDUCE];
	double gamma_row = alpha_beta_gamma[2][RECURSIVE_DOUBLING_ALL_REDUCE];
	double alpha_column = alpha_beta_gamma[0][RECURSIVE_DOUBLING_ALL_REDUCE];
	double beta_column = alpha_beta_gamma[1][RECURSIVE_DOUBLING_ALL_REDUCE];
	double gamma_column =  alpha_beta_gamma[2][RECURSIVE_DOUBLING_ALL_REDUCE];

	while(pow1 <= P){
		pow1 *= 2;
	}	
	pow1 /= 2;
	//now pow1 is the highest power of 2 in P
	ll Pc_opt=1;
	double t_opt=1e10;
	if(P == pow1){
		//brute force check all pow1 combinations
		for(int Pc_cand=1; Pc_cand < P; Pc_cand *= 2){
			double t_cand = hockneytime_rd(Pc_cand, m, ms, alpha_row, beta_row, gamma_row)
							+ hockneytime_rd(P/Pc_cand, m, ms, alpha_column, beta_column, gamma_column);
			if(t_cand < t_opt){
				Pc_opt = Pc_cand;
				t_opt = t_cand;
			}
		}
			
	}
	else{
		//use the derivative
        //instead, use al factors
		for(int i=0; i < NUM_FACTORS; i+=1){
			int Pc_cand = factorsP[i];
			double t_cand = hockneytime_rd(Pc_cand, m, ms, alpha_row, beta_row, gamma_row)
							+ hockneytime_rd(P/Pc_cand, m, ms, alpha_column, beta_column, gamma_column);
			if(t_cand < t_opt){
				Pc_opt = Pc_cand;
				t_opt = t_cand;
			}
		}
	}
	*Pc_ptr = Pc_opt;
	times41[RECURSIVE_DOUBLING_ALL_REDUCE][RECURSIVE_DOUBLING_ALL_REDUCE] = t_opt;
	return t_opt;
}
