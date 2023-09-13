#ifndef IREE_ERROR_H
#define IREE_ERROR_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/array.hpp>

#define IREE_ERR_MSG(mp_status, mp_message) \
{ \
    iree_status_t status = (mp_status); \
    if((status)) \
    { \
        String stringified_code(iree_status_code_string(iree_status_code((status)))); \
        String stringified_message((mp_message)); \
        Array format_array; \
        format_array.append(stringified_code); \
        ERR_PRINT(stringified_message); \
        ERR_PRINT(String("IREE code: '{1}'. IREE's error is logged to `stderr`.").format(format_array)); \
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
        String stringified_code(iree_status_code_string(iree_status_code((status)))); \
        String stringified_message((mp_message)); \
        Array format_array; \
        format_array.append(stringified_code); \
        ERR_PRINT(stringified_message); \
        ERR_PRINT(String("IREE code: '{1}'. IREE's error is logged to `stderr`.").format(format_array)); \
        iree_status_fprint(stderr, (status)); \
        iree_status_free((status)); \
        return (mp_retval); \
    } \
}

#endif//IREE_ERROR_H
