/* rcbuffer.h
 *
 * Copyright @ Ahmed Elkkalifa. 2026
 * MIT LICENSE
 *
 * rcbuffer.h provides a generic non-atomic reference counted heap buffers.
 * 
 * Usage:
 * In ONLY ONE .c file in your project define CCONTS_RCBUFFER_IMPLEMENTATION 
 * then include rcbuffer.h under the definition.
 *
 * To allocate a reference counted buffer:
 * int *mydata = rc_alloc(10 * sizeof(int)); // Allocates 10 ints
 *
 * To reallocate
 * mydata = rc_realloc(mydata, 20 * sizeof(int));
 *
 * Retain to increment the reference count:
 * rc_retain(mydata);
 *
 * Release will decrement the RC and free if count is 0:
 * rc_release(mydata); // Only frees if the ref count hits 0
 *
 * NOTE
 * Weak references will not be supported as it does not make sense for this
 * design. A weak reference will still hold the data (Aka no free is 
 * happening when sref is 0 but wref > 0)
 */ 

#ifndef CCONTS_RCBUFFER_H
#define CCONTS_RCBUFFER_H

#include <stdlib.h>

#define CCONTS_RCBUFFER_MAGIC 0xb0ded0f

typedef struct rc_header_ {
	int magic;
	int ref; 
	/* Some allocator's realloc implmentation requires the old size.
	 * dsize will hold the buffer size (Not including the header).
	 */
	size_t dsize;
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

/* void *rc_realloc(void *buf, size_t new_size)
 *
 * Reallocates an RC buffer to the new size.
 *
 * PARAMETERS
 * buf: RC buffer to reallocate
 * new_size: New buffer size in bytes
 *
 * RETURNS
 * Returns a pointer to the new;y reallocated buffer. Returns NULL if
 * realloc fails.
 */
void *rc_realloc(void *buf, size_t new_size);

// The user should be able to define the allocator they like
#ifndef CCONTS_RCBUFFER_MALLOC
#define CCONTS_RCBUFFER_MALLOC malloc
#endif
#ifndef CCONTS_RCBUFFER_FREE
#define CCONTS_RCBUFFER_FREE free
#endif
#ifndef CCONTS_RCBUFFER_REALLOC
static void *cconts_rcbuf_stdlib_realloc_wrapper(void *ptr, size_t new_size, size_t old_size) {
	(void)old_size;
	return realloc(ptr, new_size);
}
#define CCONTS_RCBUFFER_REALLOC cconts_rcbuf_stdlib_realloc_wrapper
#endif

#ifdef CCONTS_RCBUFFER_IMPLEMENTATION

void *rc_alloc(size_t bsize) {
	size_t fullsize = bsize + sizeof(cconts_rc_header);
	cconts_rc_header *ptr = CCONTS_RCBUFFER_MALLOC(fullsize);
	if(ptr == NULL) return NULL;
	ptr->magic = CCONTS_RCBUFFER_MAGIC;
	ptr->ref = 1;
	ptr->dsize = bsize;
	ptr++;
	return (void *)ptr;
}

void *rc_retain(void *buf) {
	cconts_rc_header *ptr = (cconts_rc_header *)buf;
	ptr--;
	if(ptr->magic != CCONTS_RCBUFFER_MAGIC) return NULL;
	if(ptr->ref == 0) return NULL;
	// Don't check the wref, later wref implmentations will handle it
	ptr->ref++;
	ptr++;
	return ptr;
}

void rc_release(void *buf) {
	cconts_rc_header *ptr = (cconts_rc_header *)buf;
	ptr--;
	if(ptr->magic != CCONTS_RCBUFFER_MAGIC) return;
	if(ptr->ref == 0) return;
	if(--ptr->ref == 0) {
		CCONTS_RCBUFFER_FREE(ptr);		
		return;
	};
}

void *rc_realloc(void *buf, size_t new_size) {
	cconts_rc_header *ptr = buf;
	ptr--;
	ptr = CCONTS_RCBUFFER_REALLOC(ptr, 
			new_size + sizeof(cconts_rc_header),
		       	ptr->dsize);
	if(ptr == NULL) return NULL;
	ptr->dsize = new_size;
	ptr++;
	return ptr;
}
#endif

#endif
