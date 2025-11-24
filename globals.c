#include"macros.h"



double times41[NUM_ALGOS][NUM_ALGOS];
int factorsP[MAX_FACTORS+1];						//factorsP[i] stores the list of factors for a given P. For every new P, we override the factorsP arr
int NUM_FACTORS = 0;


void find_and_store_factors(int P){
	for(int i=1; i<ceil(sqrt(P)); i++){
		if(P%i == 0){
			factorsP[NUM_FACTORS++] = i;
			factorsP[NUM_FACTORS++] = P/i;
		}
	}
}

