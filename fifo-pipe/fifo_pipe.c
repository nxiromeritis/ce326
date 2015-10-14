#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

volatile unsigned int in_pos;	// write position
volatile unsigned int out_pos;	// read position
volatile unsigned int n;		// storage capacity
volatile unsigned int bytes_written;
volatile unsigned short is_opened = 0;
volatile char *dstorage;


// initialize pipe
void pipe_init(int size) {

	is_opened = 1;
	in_pos = 0;
	out_pos = 0;
	n = size;
	bytes_written = 0;
	dstorage = (volatile char *)malloc(size*sizeof(char));

}


// read one byte from pipe and store it to c
// blocks if there is no data in pipe unless pipe_close i called
int pipe_read(char *c) {

	while ((bytes_written == 0)&&(is_opened)) {}

	if (!(is_opened)) {
		return(0);
	}

	*c = dstorage[out_pos];
	out_pos = (out_pos+1)%n;
	bytes_written--;

	return(1);
}


// write one byte to pipe
// blocks if pipe buffer is full
void pipe_write(char c) {

	while (bytes_written == n) {}

	dstorage[in_pos] = c;
	in_pos = (in_pos+1)%n;
	bytes_written++;

}


// wait for all data to be read
// and mark pipe as closed.
void pipe_close() {

	while (bytes_written != 0) {}

	is_opened = 0;

	// no volatile qualifier in free's prototype, cast needed
	free((char *)dstorage);
}
