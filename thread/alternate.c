#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void* runF(void * args){
	printf("In thread, the value is %d\n",(int)args);
	return (void *)0;
}

int main(){
	int i = 20;
	pthread_t p;
	int err;
	for(int j = 1; j <= i; j++){
		if(j % 2 == 1)
			err = pthread_create(&p, NULL, runF, (void *)&i);
		else
			printf("In main, the value is %d\n", j);
	}
	return 0;
}
