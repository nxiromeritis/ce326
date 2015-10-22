#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include "mandelCore.h"


#define WinW 300
#define WinH 300
#define ZoomStepFactor 0.5
#define ZoomIterationFactor 2

static Display *dsp = NULL;
static unsigned long curC;
static Window win;
static GC gc;

/* basic win management rountines */

static void openDisplay() {
  if (dsp == NULL) {
    dsp = XOpenDisplay(NULL);
  }
}

static void closeDisplay() {
  if (dsp != NULL) {
    XCloseDisplay(dsp);
    dsp=NULL;
  }
}

void openWin(const char *title, int width, int height) {
  unsigned long blackC,whiteC;
  XSizeHints sh;
  XEvent evt;
  long evtmsk;

  whiteC = WhitePixel(dsp, DefaultScreen(dsp));
  blackC = BlackPixel(dsp, DefaultScreen(dsp));
  curC = blackC;

  win = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 0, 0, WinW, WinH, 0, blackC, whiteC);

  sh.flags=PSize|PMinSize|PMaxSize;
  sh.width=sh.min_width=sh.max_width=WinW;
  sh.height=sh.min_height=sh.max_height=WinH;
  XSetStandardProperties(dsp, win, title, title, None, NULL, 0, &sh);

  XSelectInput(dsp, win, StructureNotifyMask|KeyPressMask);
  XMapWindow(dsp, win);
  do {
    XWindowEvent(dsp, win, StructureNotifyMask, &evt);
  } while (evt.type != MapNotify);

  gc = XCreateGC(dsp, win, 0, NULL);

}

void closeWin() {
  XFreeGC(dsp, gc);
  XUnmapWindow(dsp, win);
  XDestroyWindow(dsp, win);
}

void flushDrawOps() {
  XFlush(dsp);
}

void clearWin() {
  XSetForeground(dsp, gc, WhitePixel(dsp, DefaultScreen(dsp)));
  XFillRectangle(dsp, win, gc, 0, 0, WinW, WinH);
  flushDrawOps();
  XSetForeground(dsp, gc, curC);
}

void drawPoint(int x, int y) {
  XDrawPoint(dsp, win, gc, x, WinH-y);
  flushDrawOps();
}

void getMouseCoords(int *x, int *y) {
  XEvent evt;

  XSelectInput(dsp, win, ButtonPressMask);
  do {
    XNextEvent(dsp, &evt);
  } while (evt.type != ButtonPress);
  *x=evt.xbutton.x; *y=evt.xbutton.y;
}

/* color stuff */

void setColor(char *name) {
  XColor clr1,clr2;

  if (!XAllocNamedColor(dsp, DefaultColormap(dsp, DefaultScreen(dsp)), name, &clr1, &clr2)) {
    printf("failed\n"); return;
  }
  XSetForeground(dsp, gc, clr1.pixel);
  curC = clr1.pixel;
}

char *pickColor(int v, int maxIterations) {
  static char cname[128];

  if (v == maxIterations) {
    return("black");
  }
  else {
    sprintf(cname,"rgb:%x/%x/%x",v%64,v%128,v%256);
    return(cname);
  }
}


// needed by threads
volatile int *res;
mandel_Pars *slices;

volatile int notify = 0;	// worker notifier flag
volatile int *work_status;	// 0:not started, 1:done, 2:done and painted
volatile int done_paint;	// counts jobs that are already painted
int maxIterations;			// we could pass that as thread parameter for no global

void *pthread_work(void *arg) {
	int i=0;


	i = *((int *)arg);	//read arg

	while(1) {
		while(!(notify && (work_status[i]==0))) {}	// wait for main
		mandel_Calc(&slices[i],maxIterations,&res[i*slices[i].imSteps*slices[i].reSteps]);
		printf("\nthread %d: done\n", i);
		work_status[i] = 1; // mark as done but not painted
	}
	return(NULL);
}

int main(int argc, char *argv[]) {
  mandel_Pars pars;
  int i,j,x,y,nofslices,level;
  int xoff,yoff;
  long double reEnd,imEnd,reCenter,imCenter;

  int *tmp;
  pthread_t *worker;

  printf("\n");
  printf("This program starts by drawing the default Mandelbrot region\n");
  printf("When done, you can click with the mouse on an area of interest\n");
  printf("and the program will automatically zoom around this point\n");
  printf("\n");
  printf("Press enter to continue\n");
  getchar();

  pars.reSteps = WinW; /* never changes */
  pars.imSteps = WinH; /* never changes */

  /* default mandelbrot region */

  pars.reBeg = (long double) -2.0;
  reEnd = (long double) 1.0;
  pars.imBeg = (long double) -1.5;
  imEnd = (long double) 1.5;
  pars.reInc = (reEnd - pars.reBeg) / pars.reSteps;
  pars.imInc = (imEnd - pars.imBeg) / pars.imSteps;

  printf("enter max iterations (50): ");
  scanf("%d",&maxIterations);
  printf("enter no of slices: ");
  scanf("%d",&nofslices);

  /* adjust slices to divide win height */

  while (WinH % nofslices != 0) { nofslices++;}

  /* allocate slice parameter and result arrays */

  slices = (mandel_Pars *)malloc(sizeof(mandel_Pars)*nofslices);
  res = (volatile int *)malloc(sizeof(int)*pars.reSteps*pars.imSteps);



  // create workers
  work_status = (volatile int *)malloc(sizeof(int)*nofslices);
  tmp = (int *)malloc(sizeof(int)*nofslices);
  worker = (pthread_t *)malloc(sizeof(pthread_t)*nofslices);

  for(i=0; i<nofslices; i++) {
	  tmp[i] = i;
	  if(pthread_create(&worker[i], NULL, pthread_work, &tmp[i])) {
		  perror("pthread_create");
	  }
  }

  /* open window for drawing results */

  openDisplay();
  openWin(argv[0], WinW, WinH);

  level = 1;

  while (1) {

    clearWin();

    mandel_Slice(&pars,nofslices,slices);

    y=0;

	// create new jobs
	done_paint = 0;
	for (i=0; i<nofslices; i++) {
		work_status[i] = 0;
	}

	// notify workers
	notify = 1;

	// loop until all jobs are done AND painted
	while (done_paint != nofslices) {
	  i=0;
	  // stop at the first done and not painted work
	  while(work_status[i]!=1) {
		 i = (i+1)%nofslices;
	  }
	  work_status[i] = 2;
	  done_paint++;
	  /*printf("slice paint: %d total: %d\n",i, done_paint);*/


	  //draw
      for (j=0; j<slices[i].imSteps; j++) {
		for (x=0; x<slices[i].reSteps; x++) {
          setColor(pickColor(res[y*slices[i].reSteps+x],maxIterations));
          drawPoint(x,y);
        }
        y++;
      }

	}
	notify = 0; // all jobs are done

    /* get next focus/zoom point */

    getMouseCoords(&x,&y);
    xoff = x;
    yoff = WinH-y;

    /* adjust region and zoom factor  */

    reCenter = pars.reBeg + xoff*pars.reInc;
    imCenter = pars.imBeg + yoff*pars.imInc;
    pars.reInc = pars.reInc*ZoomStepFactor;
    pars.imInc = pars.imInc*ZoomStepFactor;
    pars.reBeg = reCenter - (WinW/2)*pars.reInc;
    pars.imBeg = imCenter - (WinH/2)*pars.imInc;

    maxIterations = maxIterations*ZoomIterationFactor;
    level++;

  }

  /* never reach this point; for cosmetic reasons */

  free(slices);
  free((void *)res);
  free((void *)work_status);
  closeWin();
  closeDisplay();

}
