#include "max_func.h"

int max_func(int* vect, int size) {
	if(size < 1)
		return -1;
	int max = vect[0];
	int i;
	for(i=1; i<size; i++){
		if(vect[i] > max)
			max = vect[i];
	}
	return max;
}
