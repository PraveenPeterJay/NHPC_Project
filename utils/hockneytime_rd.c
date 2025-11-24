#include"../macros.h"

double hockneytime_rd(ll P, ll m, ll ms, double alpha, double beta, double gamma){
	return log(P)/log(2) * (alpha *m + beta * m + gamma);  	
}
     
