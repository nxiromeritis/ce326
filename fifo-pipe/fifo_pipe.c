#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

volatile unsigned int in_pos;
volatile unsigned int out_pos;
volatile unsigned int len;
volatile unsigned short is_opened = 0;
volatile char *dstorage;

void pipe_init(int size) {

	is_opened = 1;
	len = size;
	dstorage = (char *)malloc(size*sizeof(char));

}

void pipe_close() {

	is_opened = 0;

}
