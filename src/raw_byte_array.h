#ifndef RAW_BYTE_ARRAY_H
#define RAW_BYTE_ARRAY_H

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <iree/hal/api.h>


namespace godot {

class RawByteArray {
private:
    uint8_t* data;
    iree_host_size_t length; 
    iree_host_size_t capacity;

    Error reserve(iree_host_size_t p_min_capacity);

public:
    RawByteArray();
    ~RawByteArray();

    void clean();
    void clear();
    const uint8_t* get_data() const;
    iree_host_size_t get_length() const;

    template<typename T>
    Error append_bytes(const T* p_source, iree_host_size_t p_length) {
        const iree_host_size_t data_size = sizeof(T) * p_length;
        Error err = reserve(capacity + data_size);
        if(err) return err;
        memcpy((void*)(data + length), (const void*)p_source, data_size);
        length += data_size;
        return OK;
    }

    template<typename T>
    Error append_bytes(const T& p_source) {
        return append_bytes<T>(&p_source, 1);
    }

};

} // namespace godot

#endif//RAW_BYTE_ARRAY_H