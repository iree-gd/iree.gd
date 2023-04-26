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
#include <godot_cpp/classes/image.hpp>

#include <iree/vm/api.h>
#include <iree/hal/api.h>

#include "raw_byte_array.h"

using namespace godot;

/* Interface between Godot and IREE bytecode in term of data. */
class IREEBufferView : public RefCounted {
    GDCLASS(IREEBufferView, RefCounted)

private:
    iree_hal_buffer_view_t* buffer_view;

    // Estimate the shape of the variant as if it is converted to buffer_view.
    static Array estimate_shape(const Variant& p_value);
    static Array estimate_shape(const Array& p_value);
    static Array estimate_shape(const PackedByteArray& p_value);
    static Array estimate_shape(const PackedInt32Array& p_value);
    static Array estimate_shape(const PackedInt64Array& p_value);
    static Array estimate_shape(const PackedFloat32Array& p_value);
    static Array estimate_shape(const PackedFloat64Array& p_value);
    static Array estimate_shape(const PackedVector2Array& p_value);
    static Array estimate_shape(const PackedVector3Array& p_value);
    static Array estimate_shape(const PackedColorArray& p_value);
    static Array estimate_shape(const Vector2& p_value);
    static Array estimate_shape(const Vector2i& p_value);
    static Array estimate_shape(const Vector3& p_value);
    static Array estimate_shape(const Vector3i& p_value);
    static Array estimate_shape(const Vector4& p_value);
    static Array estimate_shape(const Vector4i& p_value);
    static Array estimate_shape(const Color& p_value);
    static Array estimate_shape(const Object* p_value);
    static Array estimate_shape(const Ref<Image> p_value);

protected:
    static void _bind_methods();

public:
    enum ElementType {
        NONE              = IREE_HAL_ELEMENT_TYPE_NONE             ,
        OPAQUE_8          = IREE_HAL_ELEMENT_TYPE_OPAQUE_8         ,
        OPAQUE_16         = IREE_HAL_ELEMENT_TYPE_OPAQUE_16        ,
        OPAQUE_32         = IREE_HAL_ELEMENT_TYPE_OPAQUE_32        ,
        OPAQUE_64         = IREE_HAL_ELEMENT_TYPE_OPAQUE_64        ,
        BOOL_8            = IREE_HAL_ELEMENT_TYPE_BOOL_8           ,
        INT_4             = IREE_HAL_ELEMENT_TYPE_INT_4            ,
        SINT_4            = IREE_HAL_ELEMENT_TYPE_SINT_4           ,
        UINT_4            = IREE_HAL_ELEMENT_TYPE_UINT_4           ,
        INT_8             = IREE_HAL_ELEMENT_TYPE_INT_8            ,
        SINT_8            = IREE_HAL_ELEMENT_TYPE_SINT_8           ,
        UINT_8            = IREE_HAL_ELEMENT_TYPE_UINT_8           ,
        INT_16            = IREE_HAL_ELEMENT_TYPE_INT_16           ,
        SINT_16           = IREE_HAL_ELEMENT_TYPE_SINT_16          ,
        UINT_16           = IREE_HAL_ELEMENT_TYPE_UINT_16          ,
        INT_32            = IREE_HAL_ELEMENT_TYPE_INT_32           ,
        SINT_32           = IREE_HAL_ELEMENT_TYPE_SINT_32          ,
        UINT_32           = IREE_HAL_ELEMENT_TYPE_UINT_32          ,
        INT_64            = IREE_HAL_ELEMENT_TYPE_INT_64           ,
        SINT_64           = IREE_HAL_ELEMENT_TYPE_SINT_64          ,
        UINT_64           = IREE_HAL_ELEMENT_TYPE_UINT_64          ,
        FLOAT_16          = IREE_HAL_ELEMENT_TYPE_FLOAT_16         ,
        FLOAT_32          = IREE_HAL_ELEMENT_TYPE_FLOAT_32         ,
        FLOAT_64          = IREE_HAL_ELEMENT_TYPE_FLOAT_64         ,
        BFLOAT_16         = IREE_HAL_ELEMENT_TYPE_BFLOAT_16        ,
        COMPLEX_FLOAT_64  = IREE_HAL_ELEMENT_TYPE_COMPLEX_FLOAT_64 ,
        COMPLEX_FLOAT_128 = IREE_HAL_ELEMENT_TYPE_COMPLEX_FLOAT_128,
    };

    static Ref<IREEBufferView> from(const Variant& p_value, ElementType p_value_type);

    void set_move_raw_buffer_view(iree_hal_buffer_view_t* p_buffer_view);
    iree_hal_buffer_view_t* get_retain_raw_buffer_view();
    iree_hal_buffer_view_t* get_assign_raw_buffer_view();

    bool is_null() const;
    Array to_array() const;
    Ref<Image> to_image(Image::Format p_format) const;
    Array to_images(Image::Format p_format) const;
    PackedByteArray extract_bytes() const;
    void clean();

    IREEBufferView();
    IREEBufferView(IREEBufferView&& m_buffer_view);
    IREEBufferView(const IREEBufferView& p_buffer_view) = delete;
    ~IREEBufferView();
};

VARIANT_ENUM_CAST(IREEBufferView::ElementType);

#endif //IREE_BUFFER_VIEW_H