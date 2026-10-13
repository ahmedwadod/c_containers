/* darray.h
 *
 * Copyright @ Ahmed Elkkalifa. 2026
 * MIT LICENSE
 *
 * darray.h provides macros to define type specific dynamic arrays.
 *
 * To declare an array type use the macro DECLARE_DARRAY_T(type, name)
 * This will result in the declaration of the da_<name>_t and da_<name>_<func>
 * functions.
 *
 * In the a .c file use the macro IMPL_DARRAY_T(type, name) to create the
 * actual function definitions.
 *
 * To provide support for A/RC buffers, the allocator interface for this 
 * library requires a retain/release functions.
 */

/* The REALLOC interface used throughout this header takes three arguments:
 *     void *realloc(void *buf, size_t new_size, size_t old_size)
 * The old_size is required by allocators whose realloc cannot recover it on
 * its own (e.g. plain stdlib). Allocators that track the size themselves
 * (the RC/ARC buffers) simply ignore it via a thin wrapper below.
 */
#ifdef CCONTS_DARRAY_CUSTOM_ALLOC
/* The user must define
 * CCONTS_DARRAY_ALLOC
 * CCONTS_DARRAY_REALLOC
 * CCONTS_DARRAY_RETAIN
 * CCONTS_DARRAY_RELEASE
 */
#elif defined(CCONTS_DARRAY_USE_RCBUFFER)
	/* rcbuffer.h must be includable */
	#include "rcbuffer.h"
	static void *cconts_darray_rc_realloc_wrapper(void *buf,
			size_t new_size, size_t old_size) {
		(void)old_size;
		return rc_realloc(buf, new_size);
	}
	#define CCONTS_DARRAY_ALLOC rc_alloc
	#define CCONTS_DARRAY_REALLOC cconts_darray_rc_realloc_wrapper
	#define CCONTS_DARRAY_RETAIN rc_retain
	#define CCONTS_DARRAY_RELEASE rc_release
#elif defined(CCONTS_DARRAY_USE_ARCBUFFER)
	/* arcbuffer.h must be includable */
	#include "arcbuffer.h"
	static void *cconts_darray_arc_realloc_wrapper(void *buf,
			size_t new_size, size_t old_size) {
		(void)old_size;
		return arc_realloc(buf, new_size);
	}
	#define CCONTS_DARRAY_ALLOC arc_alloc
	#define CCONTS_DARRAY_REALLOC cconts_darray_arc_realloc_wrapper
	#define CCONTS_DARRAY_RETAIN arc_retain
	#define CCONTS_DARRAY_RELEASE arc_release
#else
	/* Using stdlib */
	#include <stdlib.h>
	#define CCONTS_DARRAY_ALLOC malloc
	static void *cconts_darray_realloc_wrapper(void *buf,
		       	size_t new_size, size_t old_size) {
		(void)old_size;
		return realloc(buf, new_size);
	}
	#define CCONTS_DARRAY_REALLOC cconts_darray_realloc_wrapper
	static void *cconts_darray_retain_wrapper(void *buf) {
		return buf;
	}
	#define CCONTS_DARRAY_RETAIN cconts_darray_retain_wrapper
	static void cconts_darray_release_wrapper(void *buf) {
		free(buf);
	}
	#define CCONTS_DARRAY_RELEASE cconts_darray_release_wrapper
#endif

#include <assert.h>
#include <stddef.h>

#define DECLARE_DARRAY_T(type, name) 					\
typedef struct da_##name##_t {						\
	size_t len;							\
	size_t cap;							\
	type *data;							\
} da_##name##_t;							\
da_##name##_t da_##name##_new(size_t reserve);				\
void da_##name##_release(da_##name##_t *arr);				\
void da_##name##_grow_by(da_##name##_t *arr, size_t extra);		\
void da_##name##_insert(da_##name##_t *arr, type value);		\
type da_##name##_get(da_##name##_t *arr, size_t index);			\
type *da_##name##_at(da_##name##_t *arr, size_t index);			\
void da_##name##_shiftr(da_##name##_t *arr, size_t index);		\
void da_##name##_insert_at(da_##name##_t *arr, size_t index, type value);

#define IMPL_DARRAY_T(type, name) 					\
da_##name##_t da_##name##_new(size_t reserve) {				\
	size_t capacity = reserve ? (reserve + reserve / 2) : 10;	\
	da_##name##_t arr = { 0, capacity, NULL };			\
	arr.data = CCONTS_DARRAY_ALLOC(capacity * sizeof(type));	\
	assert(arr.data);						\
	return arr;							\
}									\
									\
void da_##name##_release(da_##name##_t *arr) {				\
	CCONTS_DARRAY_RELEASE(arr->data);				\
	arr->data = NULL;						\
	arr->len = 0;							\
	arr->cap = 0;							\
}									\
									\
void da_##name##_grow_by(da_##name##_t *arr, size_t extra) {		\
	if(extra == 0) { extra = 1; }					\
	size_t new_cap = arr->cap + extra;				\
	type *grown = CCONTS_DARRAY_REALLOC(arr->data,			\
		        new_cap * sizeof(type),				\
			arr->cap * sizeof(type));			\
	assert(grown);							\
	arr->data = grown;						\
	arr->cap = new_cap;						\
}									\
									\
void da_##name##_insert(da_##name##_t *arr, type value) {		\
	if(arr->len == arr->cap) {					\
		da_##name##_grow_by(arr, arr->cap/2);			\
	}								\
	arr->data[arr->len++] = value;					\
}									\
									\
type da_##name##_get(da_##name##_t *arr, size_t index) {		\
	assert(index < arr->len);					\
	return arr->data[index];					\
}									\
									\
type *da_##name##_at(da_##name##_t *arr, size_t index) {		\
	assert(index < arr->len);					\
	return &arr->data[index];					\
}									\
									\
void da_##name##_shiftr(da_##name##_t *arr, size_t index) {		\
	assert(index <= arr->len);					\
	if(arr->len == arr->cap) {					\
		da_##name##_grow_by(arr, arr->cap/2);			\
	}								\
	for(size_t i = arr->len; i > index; i--) {			\
		arr->data[i] = arr->data[i - 1];			\
	}								\
}									\
									\
void da_##name##_insert_at(da_##name##_t *arr, size_t index,		\
		type value) {						\
	assert(index <= arr->len);					\
	da_##name##_shiftr(arr, index);					\
	arr->data[index] = value;					\
	arr->len++;							\
}
