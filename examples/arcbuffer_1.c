/* Example of using arcbuffer.h 
 *
 * Copyright @ Ahmed Elkhalifa. 2026
 */

#define CCONTS_ARCBUFFER_IMPLEMENTATION
#include "arcbuffer.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#define RETAINS_PER_THREAD 4785
#define THREADS 20

void *worker(void *data) {
	for(int i = 0; i < RETAINS_PER_THREAD; i++) {
		arc_retain(data);
	}
	return NULL;
}

int main(void) {
	void *arr = arc_alloc(10);
	pthread_t thrds[THREADS] = {};
	const int total_retains = THREADS * RETAINS_PER_THREAD + 1;
	for(int i = 0; i < THREADS; i++) {
		pthread_create(&thrds[i], NULL, worker, arr);
	}
	for(int i = 0; i < THREADS; i++) {
		pthread_join(thrds[i], NULL);
	}
	cconts_arc_header *ptr = (cconts_arc_header*)arr;
	ptr--;
	printf("Expected: %d, Actual: %d\n", total_retains, ptr->ref);
	for(int i = 0; i < ptr->ref; i++) {
		arc_release(arr);
	}
	return 0;
}
