#ifndef IREE_TENSOR_H
#define IREE_TENSOR_H

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

#include <iree/vm/api.h>
#include <iree/hal/api.h>

using namespace godot;

/* Interface between Godot and IREE bytecode in term of data. */
class IREETensor : public RefCounted {
    GDCLASS(IREETensor, RefCounted)

private:
    iree_hal_buffer_view_t* buffer_view;

protected:
    static void _bind_methods();

public:
    template<iree_hal_element_type_t> struct StorageType;

    template<iree_hal_element_type_t T>
    static Ref<IREETensor> from(typename StorageType<T>::type p_data, PackedInt64Array p_dimension);

    template<iree_hal_element_type_t T>
    Error capture(typename StorageType<T>::type p_data, PackedInt64Array p_dimension);

    IREETensor();
    IREETensor(IREETensor& p_tensor);
    IREETensor(IREETensor&& p_tensor);
    ~IREETensor();

    void set_buffer_view(iree_hal_buffer_view_t* p_buffer_view);
    iree_hal_buffer_view_t* give_buffer_view();
    iree_hal_buffer_view_t* share_buffer_view();
    void release();

    bool is_null() const;
    iree_hal_buffer_view_t* borrow_buffer_view() const;
    PackedByteArray get_data() const;
    Array get_dimension() const;
};
template<> struct IREETensor::StorageType<IREE_HAL_ELEMENT_TYPE_UINT_8> {
    using type = PackedByteArray;
    static constexpr int64_t element_size = 1; // In bytes.
};
template<> struct IREETensor::StorageType<IREE_HAL_ELEMENT_TYPE_FLOAT_32> {
    using type = PackedFloat32Array;
    static constexpr int64_t element_size = 4; // In bytes.
};
template<> struct IREETensor::StorageType<IREE_HAL_ELEMENT_TYPE_FLOAT_64> {
    using type = PackedFloat64Array;
    static constexpr int64_t element_size = 8; // In bytes.
};
template<> struct IREETensor::StorageType<IREE_HAL_ELEMENT_TYPE_SINT_32> {
    using type = PackedInt32Array;
    static constexpr int64_t element_size = 4; // In bytes.
};
template<> struct IREETensor::StorageType<IREE_HAL_ELEMENT_TYPE_SINT_64> {
    using type = PackedInt64Array;
    static constexpr int64_t element_size = 8; // In bytes.
};
#endif //IREE_TENSOR_H
