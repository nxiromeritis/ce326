#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#define N 64
#define DEBUG 0


struct qs {
	int left;
	int right;
	int curr_layer;
	int max_layer;
};

volatile int a[N];


// for debugging
void trace(struct qs qs_info) {

	if (DEBUG) {
		printf("\nleft = %d\n", qs_info.left);
		printf("right = %d\n", qs_info.right);
		printf("currL = %d\n", qs_info.curr_layer);
		printf("maxL = %d\n\n", qs_info.max_layer);
		sleep(1);
	}

}


void print_array(int a[]) {
	int i;
	printf("[");
	for(i=0; i<N; i++) {
		printf("%d, ", a[i]);
	}
	printf("]\n\n");
}



void *quicksort(void *arg) {
	int pivot;
	int i,j,tmp;
	pthread_t lthread, rthread;
	struct qs qs_info;

	// the address of the struct is passed as a function parameter
	// and for this reason we dont want the same address in both qs calls
	struct qs qs_left,qs_right;

	struct qs *qs_temp;

	qs_temp = (struct qs *)arg;
	qs_info.left  = qs_temp->left;
	qs_info.right = qs_temp->right;

	qs_info.curr_layer = qs_temp->curr_layer;
	qs_left.curr_layer = qs_temp->curr_layer;
	qs_right.curr_layer = qs_temp->curr_layer;

	qs_info.max_layer = qs_temp->max_layer;
	qs_left.max_layer = qs_temp->max_layer;
	qs_right.max_layer = qs_temp->max_layer;

	trace(qs_info);

	// pivot calculation
	if (qs_info.left < qs_info.right) {
		pivot = qs_info.left;
		i = qs_info.left;
		j = qs_info.right;

		while(i<j) {
			while((a[i]<=a[pivot]) && (i < qs_info.right))
				i++;
			while(a[j] > a[pivot])
				j--;
			if (i<j){
				tmp = a[i];
				a[i] = a[j];
				a[j] = tmp;
			}
		}
		tmp = a[pivot];
		a[pivot] = a[j];
		a[j] = tmp;

		//decide wether to continue creating threads or not
		if (qs_info.curr_layer == qs_info.max_layer) {

			qs_left.left = qs_info.left;
			qs_left.right = j-1;
			quicksort(&qs_left);

			qs_right.left = j+1;
			qs_right.right = qs_info.right;
			quicksort(&qs_right);
		}
		else {
			qs_left.curr_layer++;
			qs_left.left = qs_info.left;
			qs_left.right = j-1;

			trace(qs_left);

			if(pthread_create(&lthread, NULL, quicksort, &qs_left)) {
				perror("pthread_create");
				exit(1);
			}

			qs_right.curr_layer++;
			qs_right.left = j+1;
			qs_right.right = qs_info.right;

			trace(qs_right);

			if(pthread_create(&rthread, NULL, quicksort, &qs_right)) {
				perror("pthread_create");
				exit(1);
			}

			// need for sleep here. we dont want this function to return before
			// the thread parameter reading..(function ends-> local variables lost)
			sleep(1);
		}
	}
	return(NULL);
}



int is_sorted() {
	int i;

	for(i=1; i<N; i++)
		if (a[i] < a[i-1])
			return(0);
	return(1);
}



int main(int argc, char *argv[]) {
	int i;
	struct qs qs_info;

	// read max recursion tree threaded layer
	if(argc!=2) {
		printf("Use: %s <threaded_tree_depth>", argv[0]);
		return(0);
	}
	else {
		qs_info.max_layer = atoi(argv[1]);
		if (!(qs_info.max_layer>0)) {
			printf("Argument must be positive integer\n");
			return(0);
		}
	}

	srand(time(NULL));
	for (i=0; i<N; i++) {
		a[i] = rand()%101;
	}

	print_array((int *)a);

	qs_info.left = 0;
	qs_info.right = N-1;
	qs_info.curr_layer = 0;

	printf("main: calling qs\n");
	quicksort(&qs_info);

	while (!is_sorted()) {}
	print_array((int *)a);


	return(0);
}
