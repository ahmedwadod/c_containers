# C Containers

Header-only STB-style dynamic containers.

**Available Containers**:
- `rcbuffer.h`: Non-atomic Reference Counted Generic Buffer

**Coming Soon**:
- `arcbuffer.h`: Atmoic Reference Counted Generic Buffer
- `string.h`: Strings
- `darray.h`: Dynamic Array

## Usage Examples
For more examples see `examples/` directory.

### RC Buffer
```c
#define RCBUFFER_IMPLEMENTATION
#include "rcbuffer.h"

void main(void) {
    /* Allocate a 10 element int array */
    int *data = rc_alloc(10 * sizeof(int));

    /* Retain */
    foo(rc_retain(data));

    /* Release */
    (void)rc_release(data);
    // Foo can still use data until it calls rc_release to free it
}
```

You can optionally define `CCONTS_RCBUFFER_MALLOC` and `CCONTS_RCBUFFER_FREE` functions to use your own allocator.
