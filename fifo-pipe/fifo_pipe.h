#ifndef __FIFO_PIPE_H__
#define __FIFO_PIPE_H__

extern volatile unsigned int in_pos;
extern volatile unsigned int out_pos;
extern volatile unsigned int n;
extern volatile unsigned int bytes_written;
extern volatile unsigned short is_opened;
extern volatile char *dstorage;

extern void pipe_init();
extern void pipe_write();
extern int pipe_read();
extern void pipe_close();

#endif
