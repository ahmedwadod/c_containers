/* Example of using rcbuffer.h 
 *
 * Copyright @ Ahmed Elkhalifa. 2026
 */

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define CCONTS_RCBUFFER_IMPLEMENTATION
#include "rcbuffer.h"

struct st {
	int *arr;
};

struct st foo(int *arr) {
	// Retain a copy
	struct st s = { rc_retain(arr) };

	return s;
}

struct st bar() {
	int *data = rc_alloc(10 * sizeof(int));
	for(int i = 0; i < 10; i++) {
		data[i] = i * 4;
	}

	struct st t = foo(data);
	// Foo retains the buffer, so this release will not free it
	rc_release(data); 
	return t;
};

bool unsafe_free_check(void *buf) {
	cconts_rc_header *ptr = buf;
	ptr--;
	return ptr->magic != CCONTS_RCBUFFER_MAGIC || 
		ptr->ref == 0 ;
};

int main(void) {
	struct st t = bar();
	assert(t.arr[9] == 4 * 9);
	t.arr[9] = 7070707;
	for(int i = 0; i < 10; i++) {
		printf("arr[%d] = %d\n", i, t.arr[i]);
	}
	t.arr = rc_realloc(t.arr, 20 * sizeof(int));
	for(int i = 0; i < 20; i++) {
		t.arr[i] = i * 8;
		printf("arr[%d] = %d\n", i, t.arr[i]);
	}
	rc_release(t.arr); // This will free it
	assert(unsafe_free_check(t.arr));
	return 0;
}
