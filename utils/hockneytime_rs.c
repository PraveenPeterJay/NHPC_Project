#include"../macros.h"

double hockneytime_rs(ll P, ll m, ll ms, double alpha, double beta, double gamma){
	return (P + m/(ms * P) -2) * (alpha + beta * ms + gamma * ms) + (P-1)*(alpha + beta * m/P); 
}
