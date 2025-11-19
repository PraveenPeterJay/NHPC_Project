#include"../macros.h"

double hockeytime_rab(ll P, ll m, ll ms, double alpha, double beta, double gamma){
	return  2.0*log(P)/log(2)*alpha + (P-1)/(double)P * (2*beta*m + gamma*m);
}
