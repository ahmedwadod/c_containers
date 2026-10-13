# C Containers

Header-only, [stb-style](https://github.com/nothings/stb) generic containers for C.
Each header is a single file you drop into your project; no build system
integration or external dependencies required.

**Available containers**:
- `darray.h`: Generic dynamic array
- `rcbuffer.h`: Non-atomic reference counted generic buffer
- `arcbuffer.h`: Atomic reference counted generic buffer

**Coming soon**:
- `optional.h`: Optionals

All runnable examples below live in [`examples/`](examples/) and build with
its [`Makefile`](examples/Makefile) (`make all`).

---

## darray.h — Dynamic Array

`darray.h` is different from the other containers in this repo: instead of
one fixed set of functions, it is a *generator*. You invoke a macro with
the element type and a name, and it expands into a dedicated array type
(`da_<name>_t`) plus a full family of `da_<name>_*` functions for that
type. This gives you type-safe dynamic arrays without casting through
`void *`.

### Quick start (single file)

```c
#include "darray.h"

/* Declares da_ints_t and the da_ints_* function prototypes. */
DECLARE_DARRAY_T(int, ints)
/* Defines the da_ints_* function bodies. Do this once per type. */
IMPL_DARRAY_T(int, ints)

int main(void) {
    da_ints_t nums = da_ints_new(0);   /* 0 = use the default capacity */

    da_ints_insert(&nums, 10);
    da_ints_insert(&nums, 20);
    da_ints_insert_at(&nums, 1, 15);   /* -> 10, 15, 20 */

    int middle = da_ints_get(&nums, 1);      /* 15 */
    *da_ints_at(&nums, 0) = 99;               /* mutate in place */

    da_ints_release(&nums);
    return 0;
}
```

You can call `DECLARE_DARRAY_T`/`IMPL_DARRAY_T` as many times as you like
with different `(type, name)` pairs in the same translation unit to get
several independent array types (see
[`examples/darray_2.c`](examples/darray_2.c) for `int` and a `point_t`
struct side by side).

### Generated API

For `DECLARE_DARRAY_T(type, name)`, the generated type and functions are:

| Signature | Description |
| --- | --- |
| `da_<name>_t` | `{ size_t len; size_t cap; type *data; }` |
| `da_<name>_t da_<name>_new(size_t reserve)` | Allocate a new array. `reserve == 0` picks a default capacity. |
| `void da_<name>_release(da_<name>_t *arr)` | Free the backing storage and zero out the handle. |
| `void da_<name>_grow_by(da_<name>_t *arr, size_t extra)` | Grow capacity by `extra` elements. Called automatically; rarely needed directly. |
| `void da_<name>_insert(da_<name>_t *arr, type value)` | Append `value`, growing if necessary. |
| `type da_<name>_get(da_<name>_t *arr, size_t index)` | Bounds-checked read. |
| `type *da_<name>_at(da_<name>_t *arr, size_t index)` | Bounds-checked mutable pointer into the array. |
| `void da_<name>_insert_at(da_<name>_t *arr, size_t index, type value)` | Insert `value` at `index`, shifting later elements right. `index == len` behaves like `insert`. |

### Using darray across multiple files

In a real project you usually want one header that *declares* the array
type so any file can use it, and exactly one `.c` file that *defines* it
— the same split you'd use for any other type. `DECLARE_DARRAY_T` only
emits the struct and function prototypes (safe to include everywhere);
`IMPL_DARRAY_T` emits the function bodies (must appear in exactly one
translation unit, or you'll get duplicate-symbol link errors).

`intvec.h`:
```c
#ifndef INTVEC_H
#define INTVEC_H

#include "darray.h"

DECLARE_DARRAY_T(int, intvec)

#endif
```

`intvec.c`:
```c
#include "intvec.h"

IMPL_DARRAY_T(int, intvec)
```

Any other file just includes `intvec.h` and links against `intvec.o`:
```c
#include "intvec.h"

int main(void) {
    da_intvec_t v = da_intvec_new(4);
    da_intvec_insert(&v, 42);
    /* ... */
    da_intvec_release(&v);
    return 0;
}
```
```sh
cc -o myprogram main.c intvec.c
```

See [`examples/intvec.h`](examples/intvec.h),
[`examples/intvec.c`](examples/intvec.c) and
[`examples/darray_4.c`](examples/darray_4.c) for the full working version
of this pattern.

### Choosing the backing allocator

By default `darray.h` allocates its backing storage with plain
`malloc`/`realloc`/`free`. You can instead back it with one of the
reference-counted buffers in this repo, or with your own allocator,
by defining one of the following **before** including `darray.h`:

| Define | Effect |
| --- | --- |
| *(none)* | Default: plain stdlib `malloc`/`realloc`/`free`. |
| `CCONTS_DARRAY_USE_RCBUFFER` | Backing storage is an `rc_alloc` buffer (see [RC Buffer](#rcbufferh--non-atomic-reference-counted-buffer) below). Requires `rcbuffer.h` to be includable. |
| `CCONTS_DARRAY_USE_ARCBUFFER` | Backing storage is an `arc_alloc` buffer (see [ARC Buffer](#arcbufferh--atomic-reference-counted-buffer) below). Requires `arcbuffer.h` to be includable. |
| `CCONTS_DARRAY_CUSTOM_ALLOC` | You provide your own `CCONTS_DARRAY_ALLOC`, `CCONTS_DARRAY_REALLOC`, `CCONTS_DARRAY_RETAIN`, and `CCONTS_DARRAY_RELEASE` macros/functions. |

With `CCONTS_DARRAY_USE_RCBUFFER` (or `_ARCBUFFER`), the array's storage
is itself a refcounted buffer: you can `rc_retain`/`arc_retain`
`arr.data` directly to keep the memory alive past `da_<name>_release`.
See [`examples/darray_3.c`](examples/darray_3.c):

```c
#define CCONTS_RCBUFFER_IMPLEMENTATION
#define CCONTS_DARRAY_USE_RCBUFFER
#include "darray.h"

DECLARE_DARRAY_T(int, rcints)
IMPL_DARRAY_T(int, rcints)

int main(void) {
    da_rcints_t arr = da_rcints_new(2);
    da_rcints_insert(&arr, 42);

    rc_retain(arr.data);        /* keep the buffer alive ourselves */
    int *shared = arr.data;

    da_rcints_release(&arr);    /* drops the array's own reference */
    shared[0];                  /* still valid: 42 */

    rc_release(shared);         /* now it's actually freed */
    return 0;
}
```

A custom `CCONTS_DARRAY_REALLOC` must have this signature:
```c
void *custom_realloc(void *buf, size_t new_size, size_t old_size);
```
`old_size` exists for allocators that cannot recover the previous size on
their own (e.g. plain `realloc`); allocators that track size themselves
(like the RC/ARC buffers) simply ignore it.

---

## rcbuffer.h — Non-atomic Reference Counted Buffer

### Usage
1. Copy `rcbuffer.h` into your project.
2. In **exactly one** `.c` file, define `CCONTS_RCBUFFER_IMPLEMENTATION`
   before including the header.
3. Include `rcbuffer.h` in every other file that needs it, no
   definition required.

```c
#define CCONTS_RCBUFFER_IMPLEMENTATION
#include "rcbuffer.h"

static int *garray = NULL;

int main(void) {
    /* Allocate a 10 element int array */
    int *data = rc_alloc(10 * sizeof(int));

    /* Retain */
    garray = rc_retain(data);

    /* Release */
    rc_release(data);

    /* garray is still available, the retain above kept it alive */
    garray[1] = 10;

    /* Finally it is freed, since the ref count now hits 0 */
    rc_release(garray);
    return 0;
}
```

You can optionally define `CCONTS_RCBUFFER_MALLOC` and
`CCONTS_RCBUFFER_FREE` to use your own allocator.

You can optionally define `CCONTS_RCBUFFER_REALLOC`, given that your
realloc function has this signature:
```c
void *custom_realloc(void *buf, size_t new_size, size_t old_size);
```
The reason for this signature is that some allocators require the old
size to function (e.g. some arena allocators). RC Buffer will pass the
old size to your function.

---

## arcbuffer.h — Atomic Reference Counted Buffer

Same API and usage pattern as RC Buffer above, but the reference count is
updated atomically so it's safe to `arc_retain`/`arc_release` the same
buffer from multiple threads.

> **NOTE**: ARC Buffer only guarantees the thread-safety of the reference
> count itself. You still need your own locking (e.g. a mutex) to
> safely access the buffer's *data* from multiple threads.

### Usage
1. Copy `arcbuffer.h` into your project.
2. In **exactly one** `.c` file, define `CCONTS_ARCBUFFER_IMPLEMENTATION`
   before including the header.
3. Include `arcbuffer.h` in every other file that needs it.

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
    pthread_t thrds[THREADS] = {0};
    const int total_retains = THREADS * RETAINS_PER_THREAD + 1;

    for(int i = 0; i < THREADS; i++) {
        pthread_create(&thrds[i], NULL, worker, arr);
    }
    for(int i = 0; i < THREADS; i++) {
        pthread_join(thrds[i], NULL);
    }

    cconts_arc_header *ptr = (cconts_arc_header *)arr;
    ptr--;
    printf("Expected: %d, Actual: %d\n", total_retains, ptr->ref);

    for(int i = 0; i < ptr->ref; i++) {
        arc_release(arr);
    }
    return 0;
}
```

You can optionally define `CCONTS_ARCBUFFER_MALLOC` and
`CCONTS_ARCBUFFER_FREE` to use your own allocator.

You can optionally define `CCONTS_ARCBUFFER_REALLOC`, given that your
realloc function has this signature:
```c
void *custom_realloc(void *buf, size_t new_size, size_t old_size);
```
The reason for this signature is that some allocators require the old
size to function (e.g. some arena allocators). ARC Buffer will pass the
old size to your function.

---

## License

MIT, see [LICENSE](LICENSE).
