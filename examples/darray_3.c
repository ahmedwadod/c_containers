/* darray_3.c -- darray.h over the reference-counted buffer backend
 *
 * Copyright @ Ahmed Elkhalifa. 2026
 *
 * Defining CCONTS_DARRAY_USE_RCBUFFER before including darray.h makes the
 * array's backing storage an rc_alloc buffer, so da_<name>_new / grow_by /
 * release route through rcbuffer.h instead of stdlib malloc.
 *
 * The rcbuffer implementation itself still needs to be compiled into exactly
 * one translation unit, hence CCONTS_RCBUFFER_IMPLEMENTATION here.
 */

#define CCONTS_RCBUFFER_IMPLEMENTATION
#define CCONTS_DARRAY_USE_RCBUFFER
#include "darray.h"
#include <assert.h>
#include <stdio.h>

DECLARE_DARRAY_T(int, rcints)
IMPL_DARRAY_T(int, rcints)

int main(void) {
	/* Start tiny so the inserts below force real rc_realloc() calls. */
	da_rcints_t arr = da_rcints_new(2);
	assert(arr.data != NULL);

	for(int i = 0; i < 50; i++) {
		da_rcints_insert(&arr, i * i);
	}
	assert(arr.len == 50);
	assert(arr.cap >= arr.len);

	for(size_t i = 0; i < arr.len; i++) {
		assert(da_rcints_get(&arr, i) == (int)(i * i));
	}

	/* The backing store is a refcounted buffer: an extra retain means the
	 * first release only drops the count, so the memory survives. Capture a
	 * reference and the data length before releasing the array handle. */
	rc_retain(arr.data);
	int *shared = arr.data;
	size_t count = arr.len;

	da_rcints_release(&arr);
	assert(arr.data == NULL);   /* the array handle is cleared... */

	/* ...but our retained reference still points at live memory. */
	for(size_t i = 0; i < count; i++) {
		assert(shared[i] == (int)(i * i));
	}
	assert(shared[count - 1] == 2401);   /* 49 * 49 */

	rc_release(shared);   /* final reference gone -> actually freed */

	printf("darray_3 (rcbuffer backend) checks passed.\n");
	return 0;
}
