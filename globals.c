#include"macros.h"



double times41[NUM_ALGOS+1][NUM_ALGOS+1];
int factorsP[MAX_FACTORS+1];
int NUM_FACTORS = 0;


void find_and_store_factors(int P){
	for(int i=1; i<ceil(sqrt(P)); i++){
		if(P%i == 0){
			factorsP[NUM_FACTORS++] = i;
			factorsP[NUM_FACTORS++] = P/i;
		}
	}
}

