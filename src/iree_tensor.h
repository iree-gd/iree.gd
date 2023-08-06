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
    static Ref<IREETensor> from_bytes(PackedByteArray p_data, PackedInt64Array p_dimension);
    static Ref<IREETensor> from_float32s(PackedFloat32Array p_data, PackedInt64Array p_dimension);

    IREETensor();
    IREETensor(IREETensor& p_tensor);
    IREETensor(IREETensor&& p_tensor);
    ~IREETensor();

    Error capture_bytes(PackedByteArray p_data, PackedInt64Array p_dimension);
    Error capture_float32s(PackedFloat32Array p_data, PackedInt64Array p_dimension);
    void set_buffer_view(iree_hal_buffer_view_t* p_buffer_view);
    iree_hal_buffer_view_t* give_buffer_view();
    iree_hal_buffer_view_t* share_buffer_view();
    void release();

    bool is_null() const;
    iree_hal_buffer_view_t* borrow_buffer_view() const;
    PackedByteArray get_data() const;
    Array get_dimension() const;
};

#endif //IREE_TENSOR_H
