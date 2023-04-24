#ifndef IREE_BUFFER_VIEW_H
#define IREE_BUFFER_VIEW_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>
#include <godot_cpp/variant/color.hpp>

#include <iree/vm/api.h>
#include <iree/hal/api.h>

#include "raw_byte_array.h"

using namespace godot;

/* Interface between Godot and IREE bytecode in term of data. */
class IREEBufferView : public RefCounted {
    GDCLASS(IREEBufferView, RefCounted)

private:
    iree_hal_buffer_view_t* buffer_view;

    // Convert Godot variant and append to byte array.
    static Error push_value_into_byte_array(const Variant& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const PackedByteArray& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const PackedInt32Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const PackedInt64Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const PackedFloat32Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const PackedFloat64Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const PackedVector2Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const PackedVector3Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const PackedColorArray& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const Vector2& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const Vector2i& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const Vector3& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const Vector3i& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const Vector4& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const Vector4i& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);
    static Error push_value_into_byte_array(const Color& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes);

    IREEBufferView(iree_hal_buffer_view_t* p_buffer_view);

protected:
    static void _bind_methods();

public:
    // Estimate the dimension of the variant as if it is converted to buffer_view.
    static Array estimate_dimension(const Variant& p_value);
    static Array estimate_dimension(const Array& p_value);
    static Array estimate_dimension(const PackedByteArray& p_value);
    static Array estimate_dimension(const PackedInt32Array& p_value);
    static Array estimate_dimension(const PackedInt64Array& p_value);
    static Array estimate_dimension(const PackedFloat32Array& p_value);
    static Array estimate_dimension(const PackedFloat64Array& p_value);
    static Array estimate_dimension(const PackedVector2Array& p_value);
    static Array estimate_dimension(const PackedVector3Array& p_value);
    static Array estimate_dimension(const PackedColorArray& p_value);
    static Array estimate_dimension(const Vector2& p_value);
    static Array estimate_dimension(const Vector2i& p_value);
    static Array estimate_dimension(const Vector3& p_value);
    static Array estimate_dimension(const Vector3i& p_value);
    static Array estimate_dimension(const Vector4& p_value);
    static Array estimate_dimension(const Vector4i& p_value);
    static Array estimate_dimension(const Color& p_value);
    // Convert Godot value to raw buffer view. Caller will need to release it after use.
    static iree_hal_buffer_view_t* to_raw_buffer_view(const Variant& p_value, iree_hal_element_type_t p_value_type);
    // Convert IREE buffer view to Godot array.
    static Array to_array(const iree_hal_buffer_view_t* p_buffer_view);
    // Extract bytes from buffer views, abandon dimensions.
    static PackedByteArray extract_bytes(const iree_hal_buffer_view_t* p_buffer_view);
    // Group elements in array into arrays.
    static Array group_elements(const Array& p_array, uint64_t p_element_per_group);

    bool is_null() const;
    Array to_array() const;
    PackedByteArray extract_bytes() const;
    void set_raw_buffer_view(iree_hal_buffer_view_t* p_buffer_view);

    IREEBufferView();
    IREEBufferView(IREEBufferView&& m_buffer_view);
    IREEBufferView(const IREEBufferView& p_buffer_view) = delete;
    ~IREEBufferView();
};

#endif //IREE_BUFFER_VIEW_H