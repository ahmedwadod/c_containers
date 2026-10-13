/* intvec.c -- the one and only definition of the da_intvec_* functions
 *
 * Copyright @ Ahmed Elkhalifa. 2026
 *
 * IMPL_DARRAY_T must appear in exactly one .c file; it emits the function
 * bodies declared by DECLARE_DARRAY_T in intvec.h. Linking this object with
 * any number of other units that only include intvec.h gives them the type
 * without duplicate-definition errors.
 */

#include "intvec.h"

IMPL_DARRAY_T(int, intvec)
