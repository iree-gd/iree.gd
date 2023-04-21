#include "raw_byte_array.h"

#include <godot_cpp/core/error_macros.hpp>

#define INIT_CAPACITY 10

using namespace godot;

Error RawByteArray::reserve(iree_host_size_t p_min_capacity) {
    iree_host_size_t new_capacity = capacity == 0? INIT_CAPACITY : capacity;
    while(new_capacity < p_min_capacity) new_capacity *= 2;
    if(new_capacity == capacity) return OK;
    if((data = (uint8_t*)realloc(data, new_capacity)) == nullptr) {
        ERR_PRINT("Unable to allocate memory for RawByteArray.");
        clean();
        return FAILED;
    }
    capacity = new_capacity;
    return OK;
}

void RawByteArray::clean() {
    if(data != nullptr) {
        free(data);
        data = nullptr;
    }
    length = 0;
    capacity = 0;
}

void RawByteArray::clear() {
    length = 0;
}

const uint8_t* RawByteArray::get_data() const {
    return data;
}

iree_host_size_t RawByteArray::get_length() const {
    return length;
}

RawByteArray::RawByteArray()
:
    data(nullptr),
    length(0),
    capacity(0)
{}

RawByteArray::~RawByteArray() { clean(); }