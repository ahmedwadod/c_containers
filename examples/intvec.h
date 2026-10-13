/* intvec.h -- a reusable "vector of int" type for the project
 *
 * Copyright @ Ahmed Elkhalifa. 2026
 *
 * This is the pattern for sharing a darray type across translation units:
 * put DECLARE_DARRAY_T in a header (it emits only the struct + prototypes,
 * no function bodies) and IMPL_DARRAY_T in exactly one .c file. Every file
 * that wants to use the type just includes this header.
 */

#ifndef INTVEC_H
#define INTVEC_H

#include "darray.h"

/* Declares da_intvec_t and the da_intvec_* function prototypes. */
DECLARE_DARRAY_T(int, intvec)

#endif /* INTVEC_H */
