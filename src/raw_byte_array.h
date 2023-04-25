#ifndef RAW_BYTE_ARRAY_H
#define RAW_BYTE_ARRAY_H

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/classes/image.hpp>

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
    Error append(const T* p_source, iree_host_size_t p_length) {
        const iree_host_size_t data_size = sizeof(T) * p_length;
        Error err = reserve(capacity + data_size);
        if(err) return err;
        memcpy((void*)(data + length), (const void*)p_source, data_size);
        length += data_size;
        return OK;
    }

    template<typename T>
    Error append(const T& p_source) {
        return append<T>(&p_source, 1);
    }

    Error append(const Variant& p_value, iree_hal_element_type_t p_value_type);
    Error append(const Array& p_value, iree_hal_element_type_t p_value_type);
    Error append(const PackedByteArray& p_value, iree_hal_element_type_t p_value_type);
    Error append(const PackedInt32Array& p_value, iree_hal_element_type_t p_value_type);
    Error append(const PackedInt64Array& p_value, iree_hal_element_type_t p_value_type);
    Error append(const PackedFloat32Array& p_value, iree_hal_element_type_t p_value_type);
    Error append(const PackedFloat64Array& p_value, iree_hal_element_type_t p_value_type);
    Error append(const PackedVector2Array& p_value, iree_hal_element_type_t p_value_type);
    Error append(const PackedVector3Array& p_value, iree_hal_element_type_t p_value_type);
    Error append(const PackedColorArray& p_value, iree_hal_element_type_t p_value_type);
    Error append(const Vector2& p_value, iree_hal_element_type_t p_value_type);
    Error append(const Vector2i& p_value, iree_hal_element_type_t p_value_type);
    Error append(const Vector3& p_value, iree_hal_element_type_t p_value_type);
    Error append(const Vector3i& p_value, iree_hal_element_type_t p_value_type);
    Error append(const Vector4& p_value, iree_hal_element_type_t p_value_type);
    Error append(const Vector4i& p_value, iree_hal_element_type_t p_value_type);
    Error append(const Color& p_value, iree_hal_element_type_t p_value_type);
    Error append(const Object* p_value, iree_hal_element_type_t p_value_type);
    Error append(const Ref<Image> p_value, iree_hal_element_type_t p_value_type);
};

} // namespace godot

#endif//RAW_BYTE_ARRAY_H