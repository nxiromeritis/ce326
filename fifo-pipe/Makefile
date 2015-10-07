CC = gcc
CFLAGS = -Wall -g
OBJ = fifo_pipe.o fifotest.o

all: fifotest

fifotest: $(OBJ)
	$(CC) $(OBJ) -o fifotest -pthread

%.o: %.c fifo_pipe.h
	$(CC) $(CFLAGS) -c $< -pthread

open:
	vim *.c *.h Makefile -p

.PHONY: clean
clean:
	@echo Deleting executable and .o files...
	rm -f fifotest *.o
