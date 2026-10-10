# C Containers

Header-only STB-style dynamic containers.

**Available Containers**:
- `rcbuffer.h`: Non-atomic Reference Counted Generic Buffer

**Coming Soon**:
- `arcbuffer.h`: Atmoic Reference Counted Generic Buffer
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
