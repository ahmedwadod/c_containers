# C Containers

Header-only STB-style dynamic containers.

**Available Containers**:
- `rcbuffer.h`: Non-atomic Reference Counted Generic Buffer
- `arcbuffer.h`: Atmoic Reference Counted Generic Buffer

**Coming Soon**:
- `string.h`: Strings
- `darray.h`: Dynamic Array

## Usage
1- Copy the container you like to use .h file to your project
2- In ONLY ONE C FILE IN YOUR PROJECT define `CCONTS_<name>_IMPLEMENTATION`
3- Include the .h file after the definition
4- Include the .h file where you want to use it in your project

### RC Buffer
```c
#define CCONTS_RCBUFFER_IMPLEMENTATION
#include "rcbuffer.h"

static int *garray = NULL;

void main(void) {
    /* Allocate a 10 element int array */
    int *data = rc_alloc(10 * sizeof(int));

    /* Retain */
    garray = rc_retain(data);

    /* Release */
    (void)rc_release(data);
    
    /* garray is still available */
    garray[1] = 10;

    /* Finally it is being freed */
    rc_release(garray);
}
```

You can optionally define `CCONTS_RCBUFFER_MALLOC` and `CCONTS_RCBUFFER_FREE` functions to use your own allocator.

You can optionally define `CCONTS_RCBUFFER_REALLOC` given that your realloc function has this signture:
```c
void *custom_realloc(void *buf, size_t new_size, size_t old_size);
```
The reason for this singture is that some allocators require the old size to function (Ex. Some arena allocators). RC Buffer will make use of it and will pass the old size to your function.

### Atomic RC Buffer
```c
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
```

**NOTE** ARC Buffers only gaurantees the thread-safety of the reference count, you will need to use locking mechanisims (Ex. mutexes) to access the actual data.

You can optionally define `CCONTS_ARCBUFFER_MALLOC` and `CCONTS_ARCBUFFER_FREE` functions to use your own allocator.

You can optionally define `CCONTS_ARCBUFFER_REALLOC` given that your realloc function has this signture:
```c
void *custom_realloc(void *buf, size_t new_size, size_t old_size);
```
The reason for this singture is that some allocators require the old size to function (Ex. Some arena allocators). ARC Buffer will make use of it and will pass the old size to your function.
