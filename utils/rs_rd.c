#include"../macros.h"




//typedef double (*getTimeandPc)(ll, ll, double, double, double, ll*);
typedef long long ll;

double rs_rd(long long P, long long m, long long ms, double alpha, double beta, double gamma, ll * Pc_ptr){
	long long pow1 = 1;
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
