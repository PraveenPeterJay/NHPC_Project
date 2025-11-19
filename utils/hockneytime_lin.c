#include"../macros.h"

double hockeytime_lin(ll P, ll m, ll ms, double alpha, double beta, double gamma){
	return (P-1)*(alpha*2 + beta*m*2 + gamma*m);
}
