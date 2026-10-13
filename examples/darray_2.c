/* darray_2.c -- two array types in one translation unit
 *
 * Copyright @ Ahmed Elkhalifa. 2026
 *
 * Shows that the `name` tag keeps distinct element types fully separate:
 * da_ints_t and da_points_t coexist with their own function families.
 * Uses the default stdlib backend.
 */

#include "darray.h"
#include <assert.h>
#include <stdio.h>

typedef struct {
	int x;
	int y;
} point_t;

DECLARE_DARRAY_T(int, ints)
IMPL_DARRAY_T(int, ints)

DECLARE_DARRAY_T(point_t, points)
IMPL_DARRAY_T(point_t, points)

int main(void) {
	/* An int array: fill, sum, and mutate in place. */
	da_ints_t nums = da_ints_new(0);   /* reserve 0 -> default capacity */
	long sum = 0;
	for(int i = 1; i <= 100; i++) {
		da_ints_insert(&nums, i);
		sum += i;
	}
	assert(nums.len == 100);

	long check = 0;
	for(size_t i = 0; i < nums.len; i++) {
		check += da_ints_get(&nums, i);
	}
	assert(check == sum);
	assert(sum == 5050);

	/* Double every value through the mutable at() pointer. */
	for(size_t i = 0; i < nums.len; i++) {
		*da_ints_at(&nums, i) *= 2;
	}
	assert(da_ints_get(&nums, 0) == 2);
	assert(da_ints_get(&nums, 99) == 200);
	printf("ints: len=%zu first=%d last=%d\n",
			nums.len,
			da_ints_get(&nums, 0),
			da_ints_get(&nums, nums.len - 1));

	/* A struct array alongside, proving the two families are independent. */
	da_points_t path = da_points_new(4);
	da_points_insert(&path, (point_t){ 0, 0 });
	da_points_insert(&path, (point_t){ 3, 4 });
	da_points_insert_at(&path, 1, (point_t){ 1, 1 });   /* splice in middle */

	assert(path.len == 3);
	assert(da_points_get(&path, 0).x == 0 && da_points_get(&path, 0).y == 0);
	assert(da_points_get(&path, 1).x == 1 && da_points_get(&path, 1).y == 1);
	assert(da_points_get(&path, 2).x == 3 && da_points_get(&path, 2).y == 4);
	printf("points: len=%zu mid=(%d,%d)\n",
			path.len,
			da_points_get(&path, 1).x,
			da_points_get(&path, 1).y);

	da_ints_release(&nums);
	da_points_release(&path);

	printf("darray_2 checks passed.\n");
	return 0;
}
