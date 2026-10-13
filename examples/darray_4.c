/* darray_4.c -- using a darray type that lives in another translation unit
 *
 * Copyright @ Ahmed Elkhalifa. 2026
 *
 * This file never calls IMPL_DARRAY_T. It just includes intvec.h for the
 * declarations and links against intvec.o for the implementations. Build:
 *
 *     cc -I../ -std=c11 -o darray_4 darray_4.c intvec.c
 */

#include "intvec.h"
#include <assert.h>
#include <stdio.h>

/* Takes the type by pointer like any ordinary struct -- no macros needed
 * here, the functions already exist in intvec.o. */
static int sum(const da_intvec_t *v) {
	int total = 0;
	for(size_t i = 0; i < v->len; i++) {
		total += v->data[i];
	}
	return total;
}

int main(void) {
	da_intvec_t v = da_intvec_new(4);

	for(int i = 1; i <= 5; i++) {
		da_intvec_insert(&v, i * 10);
	}
	assert(v.len == 5);
	assert(sum(&v) == 150);

	/* Splice a value into the middle and read it back. */
	da_intvec_insert_at(&v, 2, 999);
	assert(v.len == 6);
	assert(da_intvec_get(&v, 2) == 999);
	assert(da_intvec_get(&v, 3) == 30);   /* shifted right */

	printf("intvec: ");
	for(size_t i = 0; i < v.len; i++) {
		printf("%d ", da_intvec_get(&v, i));
	}
	printf("(sum=%d)\n", sum(&v));

	da_intvec_release(&v);
	assert(v.data == NULL);

	printf("darray_4 (multi-file project) checks passed.\n");
	return 0;
}
