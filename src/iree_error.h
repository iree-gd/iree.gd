#ifndef IREE_ERROR_H
#define IREE_ERROR_H

#include <iree/base/api.h>

#include <godot_cpp/variant/utility_functions.hpp>

#define IREE_ERR_MSG(mp_status, mp_message) \
{ \
    iree_status_t status = (mp_status); \
    if((status)) \
    { \
        ERR_PRINT((mp_message)); \
        ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(status)))); \
        iree_status_fprint(stderr, (status)); \
        iree_status_free((status)); \
        return; \
    } \
}

#define IREE_ERR_V_MSG(mp_status, mp_retval, mp_message) \
{ \
    iree_status_t status = (mp_status); \
    if((status)) \
    { \
        ERR_PRINT((mp_message)); \
        ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(status)))); \
        iree_status_fprint(stderr, (status)); \
        iree_status_free((status)); \
        return (mp_retval); \
    } \
}

#endif//IREE_ERROR_H
