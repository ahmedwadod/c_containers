/* arcbuffer.h
 *
 * Copyright @ Ahmed Elkkalifa. 2026
 * MIT LICENSE
 *
 * arcbuffer.h provides a generic atomic reference counted heap buffers.
 *
 * NOTICE
 * For thread-safe operations you will need to use locking mechanisim.
 * This library ONLY gaurantees the thread-safety of the reference count 
 * and not the data.
 * 
 * Usage:
 * In ONLY ONE .c file in your project define CCONTS_ARCBUFFER_IMPLEMENTATION 
 * then include arcbuffer.h under the definition.
 *
 * To allocate a reference counted buffer:
 * int *mydata = arc_alloc(10 * sizeof(int)); // Allocates 10 ints
 *
 * To reallocate
 * mydata = arc_realloc(mydata, 20 * sizeof(int));
 *
 * Retain to increment the reference count:
 * arc_retain(mydata);
 *
 * Release will decrement the ARC and free if count is 0:
 * arc_release(mydata); // Only frees if the ref count hits 0
 *
 * NOTE
 * Weak references will not be supported as it does not make sense for this
 * design. A weak reference will still hold the data (Aka no free is 
 * happening when sref is 0 but wref > 0)
 */ 

#ifndef CCONTS_ARCBUFFER_H
#define CCONTS_ARCBUFFER_H

#include <stdatomic.h>
#include <stdlib.h>

#define CCONTS_ARCBUFFER_MAGIC 0xa701c0f

typedef struct arc_header_ {
	int magic;
	atomic_int ref; 
	/* Some allocator's realloc implmentation requires the old size.
	 * dsize will hold the buffer size (Not including the header).
	 */
	size_t dsize;
} cconts_arc_header;

/* void *arc_alloc(size_t bsize)
 *
 * Allocates a new atomic reference counted heap buffer.
 *
 * PARAMETERS
 * bsize: Buffer size in bytes
 *
 * RETURNS
 * Returns a pointer to the newly allocated buffer or NULL on failure. The
 * returned address is of the start of the data.
 */
void *arc_alloc(size_t bsize);

/* void *arc_retain(void *buf);
 *
 * Used to increment the reference count.
 *
 * PARAMETERS
 * buf: Pointer to the buffer (Return value of arc_alloc)
 *
 * RETURNS
 * Returns a pointer to the buffer or NULL if retainning failed. This
 * function can fail in 2 scenarios: The buffer has been released, or that
 * the buffer was not created by arc_alloc in the first place.
 */
void *arc_retain(void *buf);

/* void arc_release(void *buf)
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
void arc_release(void *buf);

/* void *arc_realloc(void *buf, size_t new_size)
 *
 * Reallocates an ARC buffer to the new size.
 *
 * PARAMETERS
 * buf: ARC buffer to reallocate
 * new_size: New buffer size in bytes
 *
 * RETURNS
 * Returns a pointer to the newly reallocated buffer. Returns NULL if
 * realloc fails.
 */
void *arc_realloc(void *buf, size_t new_size);

// The user should be able to define the allocator they like
#ifndef CCONTS_ARCBUFFER_MALLOC
#define CCONTS_ARCBUFFER_MALLOC malloc
#endif
#ifndef CCONTS_ARCBUFFER_FREE
#define CCONTS_ARCBUFFER_FREE free
#ifndef CCONTS_ARCBUFFER_REALLOC
static void *cconts_arcbuf_stdlib_realloc_wrapper(void *ptr, size_t new_size, size_t old_size) {
	(void)old_size;
	return realloc(ptr, new_size);
}
#define CCONTS_ARCBUFFER_REALLOC cconts_arcbuf_stdlib_realloc_wrapper
#endif
#endif

#ifdef CCONTS_ARCBUFFER_IMPLEMENTATION

void *arc_alloc(size_t bsize) {
	size_t fullsize = bsize + sizeof(cconts_arc_header);
	cconts_arc_header *ptr = CCONTS_ARCBUFFER_MALLOC(fullsize);
	if(ptr == NULL) return NULL;
	ptr->magic = CCONTS_ARCBUFFER_MAGIC;
	ptr->ref = 1;
	ptr->dsize = bsize;
	ptr++;
	return (void *)ptr;
}

void *arc_retain(void *buf) {
	int one = 1;
	cconts_arc_header *ptr = (cconts_arc_header *)buf;
	ptr--;
	if(ptr->magic != CCONTS_ARCBUFFER_MAGIC) return NULL;
	if(ptr->ref == 0) return NULL;
	if(atomic_compare_exchange_strong(&ptr->ref, &one, 2)) {}
	else if(atomic_fetch_add(&ptr->ref, 1) < 1) return NULL;
	ptr++;
	return ptr;
}

void arc_release(void *buf) {
	cconts_arc_header *ptr = (cconts_arc_header *)buf;
	ptr--;
	if(ptr->magic != CCONTS_ARCBUFFER_MAGIC) return;
	if(atomic_fetch_sub(&ptr->ref, 1) == 1) {
		CCONTS_ARCBUFFER_FREE(ptr);		
		return;
	};
}

void *arc_realloc(void *buf, size_t new_size) {
	cconts_arc_header *ptr = buf;
	ptr--;
	ptr = CCONTS_ARCBUFFER_REALLOC(ptr, 
			new_size + sizeof(cconts_arc_header),
		       	ptr->dsize);
	if(ptr == NULL) return NULL;
	ptr->dsize = new_size;
	return ptr;
}
#endif

#endif
