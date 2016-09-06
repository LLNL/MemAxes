#pragma once

#include <Mitos/Mitos.h>

#define MITOS_ALLOC(dtype, varname, xdim, ydim, zdim) \
    dtype * varname = (dtype*)malloc(xdim*ydim*zdim*sizeof(dtype)); \
    { \
        size_t _dims[] = {xdim, ydim, zdim}; \
        Mitos_add_symbol(#varname, varname, sizeof(dtype), _dims, 3); \
    }
