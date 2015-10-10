#include "FIFOpipethreading.h"

#ifndef STDIO
	#define STDIO 
	#include <stdio.h>
#endif
	
#ifndef STDLIB
	#define STDLIB
	#include <stdlib.h>
#endif
	
#ifndef PTHREAD
	#define PTHREAD 
	#include <pthread.h>
#endif
#ifndef ERRNO
	#define ERRNO
	#include <errno.h>
#endif
#include <time.h>
#include <unistd.h>

extern volatile unsigned short done_reading;
extern volatile unsigned short done_writing;

void *pthread_reader(void *arg) {
	char c;
	int i = 1;

	printf("\nThread is ready to read\n");
	while(pipe_read(&c)) {
		printf("\t\t%d:thread got %c\n", i++, c);
		
		//
		if (done_reading)
			break;
	}
	printf("\nPipe is closed\n");
	
	return(NULL);
}


int main(int argc, char* argv[]){
	pthread_t thread1;
	int size, bytes;
	int i;
	char *c;
	
	done_reading = 0;
	done_writing = 0;
	
	do{
		printf("Give size of the Pipe in bytes:");
		scanf(" %d", &size);
	}while (size <= 0);
	
	do{
		printf("Give bytes of the string:");
		scanf(" %d", &bytes);
	}while (bytes <= 0);
	
	c = (char *) malloc((bytes + 1) * sizeof(char)); // +1 for a \0
	srand(time(NULL));
	for (i = 0; i < bytes; i++) {
		c[i] = rand()%94 + 33;
	}
	c[i] = '\0';
	printf("Characters to send: %s\n\n", c);
	
	//create thread
	if (pthread_create(&thread1, NULL, pthread_reader, &c)) {
		perror("pthread_create");
		exit(1);
	}
	printf("main: thread created\n");

	//pipe init might be dangerous here..thread might find pipe closed and exit
	pipe_init(size);
	for (i = 0; i < bytes; i++) {
		pipe_write(c[i]);
		printf("main: wrote %c\n", c[i]);
	}
	
	//informs thread that main thread is done writing to the pipe
	done_writing = 1;
	
	//waits until thread finish reading from pipe
	while (done_reading != 1);
	
	pipe_close();
	free(c);

	return(0);
}
