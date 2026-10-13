/* darray_1.c -- sanity check for darray.h
 *
 * Copyright @ Ahmed Elkhalifa. 2026
 *
 * Exercises the core array logic using the default stdlib backend.
 * A `char` array type is defined with DECLARE_DARRAY_T / IMPL_DARRAY_T,
 * producing da_chars_t and its da_chars_* functions.
 */

#include "darray.h"
#include <assert.h>
#include <stdio.h>

DECLARE_DARRAY_T(char, chars)
IMPL_DARRAY_T(char, chars)

static void print_arr(const char *label, da_chars_t *arr) {
	printf("%s (len=%zu cap=%zu): ", label, arr->len, arr->cap);
	for(size_t i = 0; i < arr->len; i++) {
		printf("%c", arr->data[i]);
	}
	printf("\n");
}

int main(void) {
	/* new() with a small reserve exercises the "cap/2 == 0" growth path. */
	da_chars_t arr = da_chars_new(1);
	assert(arr.len == 0);
	assert(arr.cap >= 1);
	assert(arr.data != NULL);

	/* Append enough to force several reallocations. */
	const char *src = "abcdefghij";
	for(size_t i = 0; src[i]; i++) {
		da_chars_insert(&arr, src[i]);
	}
	assert(arr.len == 10);
	assert(arr.cap >= arr.len);
	print_arr("after inserts", &arr);
	for(size_t i = 0; i < arr.len; i++) {
		assert(da_chars_get(&arr, i) == src[i]);
	}

	/* at() returns a mutable pointer. */
	*da_chars_at(&arr, 0) = 'A';
	assert(da_chars_get(&arr, 0) == 'A');

	/* insert_at in the middle: "Abcdefghij" -> "AbXcdefghij" */
	da_chars_insert_at(&arr, 2, 'X');
	assert(arr.len == 11);
	assert(da_chars_get(&arr, 2) == 'X');
	assert(da_chars_get(&arr, 3) == 'c');   /* old element shifted right */
	assert(da_chars_get(&arr, 10) == 'j');  /* last original element */
	print_arr("after insert_at(2,'X')", &arr);

	/* insert_at the front. */
	da_chars_insert_at(&arr, 0, '<');
	assert(da_chars_get(&arr, 0) == '<');
	assert(da_chars_get(&arr, 1) == 'A');

	/* insert_at the end (index == len) must behave like append. */
	size_t end = arr.len;
	da_chars_insert_at(&arr, end, '>');
	assert(arr.len == end + 1);
	assert(da_chars_get(&arr, end) == '>');
	print_arr("after front+end inserts", &arr);

	const char *expect = "<AbXcdefghij>";
	assert(arr.len == 13);
	for(size_t i = 0; i < arr.len; i++) {
		assert(da_chars_get(&arr, i) == expect[i]);
	}

	da_chars_release(&arr);
	assert(arr.data == NULL);
	assert(arr.len == 0);
	assert(arr.cap == 0);

	printf("All darray sanity checks passed.\n");
	return 0;
}
