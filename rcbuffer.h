/* rcbuffer.h
 *
 * Copyright @ Ahmed Elkkalifa. 2026
 * MIT LICENSE
 *
 * rcbuffer.h provides a generic non-atomic reference counted heap buffers.
 * 
 * Usage:
 * In a SINGLE .c file in your project define RCBUFFER_IMPLEMENTATION then
 * include rcbuffer.h
 *
 * To allocate a reference counted buffer:
 * int *mydata = rc_alloc(10 * sizeof(int)); // Allocates 10 ints
 *
 * Retain to increment the reference count:
 * rc_retain(mydata);
 *
 * Release will decrement the RC and free if count is 0:
 * rc_release(mydata); // Only frees if the ref count hits 0
 *
 * P.S. Weak reference support comming.
 */ 

#ifndef CCONTS_RCBUFFER_H
#define CCONTS_RCBUFFER_H

#include <stdlib.h>

#define CCONTS_RCBUFFER_MAGIC 0xb0ded0f

typedef struct rc_header_ {
	int magic;
	int sref; // Stong reference
	int wref; // Weal reference
} cconts_rc_header;

/* void *rc_alloc(size_t bsize)
 *
 * Allocates a new reference counted heap buffer.
 *
 * PARAMETERS
 * bsize: Buffer size in bytes
 *
 * RETURNS
 * Returns a pointer to the newly allocated buffer or NULL on failure. The
 * returned address is of the start of the data.
 */
void *rc_alloc(size_t bsize);

/* void *rc_retain(void *buf);
 *
 * Used to increment the reference count.
 *
 * PARAMETERS
 * buf: Pointer to the buffer (Return value of rc_alloc)
 *
 * RETURNS
 * Returns a pointer to the buffer or NULL if retainning failed. This
 * function can fail in 2 scenarios: The buffer has been released, or that
 * the buffer was not created by rc_alloc in the first place.
 */
void *rc_retain(void *buf);

/* void rc_release(void *buf)
 *
 * Decerements the reference count and frees the memory if it hits 0.
 *
 * PARAMETERS
 * buf: Pointer to the buffer
 *
 * RETURNS
 * The function does not return anything, it will fail silently in case of
 * double freeing or in case of trying to release a non-rc buffer.
 *
 * WARNING
 * Release-after-free is undefined behaviour.
 */
void rc_release(void *buf);

// The user should be able to define the allocator they like
#ifndef CCONTS_RCBUFFER_MALLOC
#define CCONTS_RCBUFFER_MALLOC malloc
#endif
#ifndef CCONTS_RCBUFFER_FREE
#define CCONTS_RCBUFFER_FREE free
#endif

#ifdef RCBUFFER_IMPLEMENTATION

void *rc_alloc(size_t bsize) {
	size_t fullsize = bsize + sizeof(cconts_rc_header);
	cconts_rc_header *ptr = CCONTS_RCBUFFER_MALLOC(fullsize);
	if(ptr == NULL) return NULL;
	ptr->magic = CCONTS_RCBUFFER_MAGIC;
	ptr->sref = 1;
	ptr->wref = 1;
	ptr++;
	return (void *)ptr;
}

void *rc_retain(void *buf) {
	cconts_rc_header *ptr = (cconts_rc_header *)buf;
	ptr--;
	if(ptr->magic != CCONTS_RCBUFFER_MAGIC) return NULL;
	if(ptr->sref == 0) return NULL;
	// Don't check the wref, later wref implmentations will handle it
	ptr->sref++;
	ptr++;
	return ptr;
}

void rc_release(void *buf) {
	cconts_rc_header *ptr = (cconts_rc_header *)buf;
	ptr--;
	if(ptr->magic != CCONTS_RCBUFFER_MAGIC) return;
	if(ptr->sref == 0) return;
	if(--ptr->sref == 0) {
		if(--ptr->wref) {
			CCONTS_RCBUFFER_FREE(ptr);		
			return;
		}
	};
}
#endif

#endif
