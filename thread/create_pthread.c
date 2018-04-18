#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

struct Books{
	char name[50];
	char author[50];
};

void print_id(struct Books* args){
	pid_t pid;
	pthread_t tid;
	pid = getpid();
	tid = pthread_self();
	printf("%s pid is %u, tid is %x\n", args->name, pid,tid);

}

void* pthread_run(void* args){
		print_id((struct Books*)args);
		return (void*)0;
}


int main(){

	struct Books book1;
	struct Books* b;
	b = &book1;
	strcpy(book1.name, "C Programming");
	strcpy(book1.author, "Wei");
	pthread_t p;
	int err;
	err = pthread_create(&p, NULL,pthread_run, (void *)b);
	if (err != 0){
		return 0;
	}
	struct Books* book2;
	strcpy(book2->name, "JAVa");
	strcpy(book2->author,"Chen");
	//printf("%s gggggg", book2->name);
	print_id(book2);
	return 0;


}

