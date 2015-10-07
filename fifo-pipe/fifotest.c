/* test1: pipe_size < bytes, make reader sleep, print what is written => pipe_write->blocking
 * test2: make writer sleep more than reader, => pipe_read-> blocking*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "fifo_pipe.h"

void *pthread_reader(void *arg) {
	char c;
	int i = 1;

	printf("\nThread is ready to read\n");
	while(pipe_read(&c)) {
		printf("%d:thread got %c\n", i++, c);
		//sleep(1);
	}
	printf("\nPipe is closed\n");

	return(NULL);
}




int main(int argc, char *argv[]) {
	pthread_t thread1;
	int pipe_size, bytes;
	char *c;
	int i;


	// read and check arguments given
	if (argc != 3) {
		printf("Invalid number of arguments.\n");
		printf("Use: \"%s <N> <K>\" (<N>:Nof bytes, <K>:Pipe size)\nExiting.\n", argv[0]);
		return(0);
	}
	else {
		bytes = atoi(argv[1]);
		pipe_size = atoi(argv[2]);
		if ((pipe_size <= 0)||(bytes <=0)) {
			printf("Invalid arguments: Both must be positive integers.\nExiting.\n");
			return(0);
		}
		printf("\nbytes = %d\npipe_size = %d\n", bytes, pipe_size);
	}


	// create a string of random characters to send
	c = (char *)malloc((bytes+1)*sizeof(char)); // +1 for a \0
	srand(time(NULL));
	for (i=0; i<bytes; i++) {
		c[i] = rand()%94 + 33;
	}
	c[i] = '\0';
	printf("Characters to send: %s\n\n", c);



	// create thread
	if (pthread_create(&thread1, NULL, pthread_reader, &c)) {
		perror("pthread_create");
	}
	printf("main: thread created\n");

	//pipe init might be dangerous here..thread might find pipe closed and exit
	pipe_init(pipe_size);
	for (i=0; i<bytes; i++) {
		pipe_write(c[i]);
		//printf("main: wrote %c\n", c[i]);
		//sleep(3);
	}


	sleep(1);
	pipe_close();
	free(c);

	return(0);
}
