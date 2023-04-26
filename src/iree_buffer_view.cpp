#include "iree_buffer_view.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include "iree.h"

#define MAX_SHAPE_RANK 20

using namespace godot;

static iree_host_size_t get_image_format_channel_count(Image::Format p_format) {
    switch (p_format) {
		case Image::Format::FORMAT_RF:
		case Image::Format::FORMAT_R8:
		case Image::Format::FORMAT_L8: {
            return 1;
        }

		case Image::Format::FORMAT_RGF:
		case Image::Format::FORMAT_RG8:
		case Image::Format::FORMAT_LA8: {
            return 2;
        }

		case Image::Format::FORMAT_RGBF:
		case Image::Format::FORMAT_RGB565:
		case Image::Format::FORMAT_RGB8: {
            return 3;
        }

		case Image::Format::FORMAT_RGBAF:
		case Image::Format::FORMAT_RGBA4444:
		case Image::Format::FORMAT_RGBA8: {
            return 4;
        }

		case Image::Format::FORMAT_ETC2_R11:
		case Image::Format::FORMAT_ETC2_R11S:
		case Image::Format::FORMAT_RH:
		case Image::Format::FORMAT_ETC2_RA_AS_RG:
		case Image::Format::FORMAT_ETC2_RG11:
		case Image::Format::FORMAT_ETC2_RG11S:
		case Image::Format::FORMAT_RGH:
		case Image::Format::FORMAT_DXT5_RA_AS_RG:
		case Image::Format::FORMAT_ETC2_RGB8:
		case Image::Format::FORMAT_ETC:
		case Image::Format::FORMAT_BPTC_RGBFU:
		case Image::Format::FORMAT_BPTC_RGBF:
		case Image::Format::FORMAT_RGBE9995:
		case Image::Format::FORMAT_RGBH:
		case Image::Format::FORMAT_RGBAH:
		case Image::Format::FORMAT_ETC2_RGB8A1:
		case Image::Format::FORMAT_ETC2_RGBA8:
		case Image::Format::FORMAT_BPTC_RGBA:
		case Image::Format::FORMAT_RGTC_RG:
		case Image::Format::FORMAT_RGTC_R:
		case Image::Format::FORMAT_DXT5:
		case Image::Format::FORMAT_DXT3:
		case Image::Format::FORMAT_DXT1:
		case Image::Format::FORMAT_ASTC_4x4:
		case Image::Format::FORMAT_ASTC_4x4_HDR:
		case Image::Format::FORMAT_ASTC_8x8:
		case Image::Format::FORMAT_ASTC_8x8_HDR:
		case Image::Format::FORMAT_MAX: {
            return 0; // Unsupported. These are madness.
        }
    }
}

void IREEBufferView::_bind_methods() {
    ClassDB::bind_static_method("IREEBufferView", D_METHOD("from", "value", "type"), &IREEBufferView::from);

    ClassDB::bind_method(D_METHOD("is_null"), &IREEBufferView::is_null);
    ClassDB::bind_method(D_METHOD("to_array"), &IREEBufferView::to_array);
    ClassDB::bind_method(D_METHOD("to_image"), &IREEBufferView::to_image);
    ClassDB::bind_method(D_METHOD("extract_bytes"), &IREEBufferView::extract_bytes);
    ClassDB::bind_method(D_METHOD("clean"), &IREEBufferView::clean);

    BIND_ENUM_CONSTANT(NONE             );
    BIND_ENUM_CONSTANT(OPAQUE_8         );
    BIND_ENUM_CONSTANT(OPAQUE_16        );
    BIND_ENUM_CONSTANT(OPAQUE_32        );
    BIND_ENUM_CONSTANT(OPAQUE_64        );
    BIND_ENUM_CONSTANT(BOOL_8           );
    BIND_ENUM_CONSTANT(INT_4            );
    BIND_ENUM_CONSTANT(SINT_4           );
    BIND_ENUM_CONSTANT(UINT_4           );
    BIND_ENUM_CONSTANT(INT_8            );
    BIND_ENUM_CONSTANT(SINT_8           );
    BIND_ENUM_CONSTANT(UINT_8           );
    BIND_ENUM_CONSTANT(INT_16           );
    BIND_ENUM_CONSTANT(SINT_16          );
    BIND_ENUM_CONSTANT(UINT_16          );
    BIND_ENUM_CONSTANT(INT_32           );
    BIND_ENUM_CONSTANT(SINT_32          );
    BIND_ENUM_CONSTANT(UINT_32          );
    BIND_ENUM_CONSTANT(INT_64           );
    BIND_ENUM_CONSTANT(SINT_64          );
    BIND_ENUM_CONSTANT(UINT_64          );
    BIND_ENUM_CONSTANT(FLOAT_16         );
    BIND_ENUM_CONSTANT(FLOAT_32         );
    BIND_ENUM_CONSTANT(FLOAT_64         );
    BIND_ENUM_CONSTANT(BFLOAT_16        );
    BIND_ENUM_CONSTANT(COMPLEX_FLOAT_64 );
    BIND_ENUM_CONSTANT(ElementType::COMPLEX_FLOAT_128);

}

Array IREEBufferView::estimate_shape(const Variant& p_value) {
    switch(p_value.get_type()) {
        case Variant::Type::ARRAY: return estimate_shape(Array(p_value));
        case Variant::Type::PACKED_BYTE_ARRAY: return estimate_shape(PackedByteArray(p_value));
        case Variant::Type::PACKED_INT32_ARRAY: return estimate_shape(PackedInt32Array(p_value));
        case Variant::Type::PACKED_INT64_ARRAY: return estimate_shape(PackedInt64Array(p_value));
        case Variant::Type::PACKED_FLOAT32_ARRAY: return estimate_shape(PackedFloat32Array(p_value));
        case Variant::Type::PACKED_FLOAT64_ARRAY: return estimate_shape(PackedFloat64Array(p_value));
        case Variant::Type::PACKED_VECTOR2_ARRAY: return estimate_shape(PackedVector2Array(p_value));
        case Variant::Type::PACKED_VECTOR3_ARRAY: return estimate_shape(PackedVector3Array(p_value));
        case Variant::Type::PACKED_COLOR_ARRAY: return estimate_shape(PackedColorArray(p_value));
        case Variant::Type::VECTOR2: return estimate_shape(Vector2(p_value));
        case Variant::Type::VECTOR2I: return estimate_shape(Vector2i(p_value));
        case Variant::Type::VECTOR3: return estimate_shape(Vector3(p_value));
        case Variant::Type::VECTOR3I: return estimate_shape(Vector3i(p_value));
        case Variant::Type::COLOR: return estimate_shape(Color(p_value));
        case Variant::Type::VECTOR4: return estimate_shape(Vector4(p_value));
        case Variant::Type::VECTOR4I: return estimate_shape(Vector4i(p_value));
        case Variant::Type::OBJECT: return estimate_shape((Object*)(p_value));
        case Variant::Type::BOOL:
        case Variant::Type::INT:
        case Variant::Type::FLOAT:
        case Variant::Type::STRING:
        case Variant::Type::RECT2:
        case Variant::Type::RECT2I:
        case Variant::Type::TRANSFORM2D:
        case Variant::Type::PLANE:
        case Variant::Type::QUATERNION:
        case Variant::Type::AABB:
        case Variant::Type::BASIS:
        case Variant::Type::TRANSFORM3D:
        case Variant::Type::PROJECTION:
        case Variant::Type::STRING_NAME:
        case Variant::Type::NODE_PATH:
        case Variant::Type::RID:
        case Variant::Type::CALLABLE:
        case Variant::Type::SIGNAL:
        case Variant::Type::DICTIONARY:
        case Variant::Type::PACKED_STRING_ARRAY:
        case Variant::Type::VARIANT_MAX:
        case Variant::Type::NIL:
        default:
            return Array();
    }

}

Array IREEBufferView::estimate_shape(const Array& p_value) {
    Array result;

    bool expecting_atomic = true;
    Array expecting_dimension; // 0 -> not expecting dimensional variant; else -> enforce the dimensional element has the same dimension.
    int64_t size = p_value.size();
    for(int64_t i = 0; i < size; i++) {
        const Variant& element = p_value[i];
        const Variant::Type element_type = element.get_type();

        if(i == 0) 
            if(!(element_type == Variant::Type::INT || element_type == Variant::Type::FLOAT)) {
                expecting_atomic = false;
                expecting_dimension = estimate_shape(element);
                if(expecting_dimension.size() == 0) return Array();
                continue;
            }

        // Expecting atomic.
        if(expecting_atomic) {
            if(element_type != Variant::Type::INT && element_type != Variant::Type::FLOAT) 
                return Array();
        }

        // Expecting array.
        else if(estimate_shape(element) != expecting_dimension) return Array();
    }       

    result.append(size);
    result.append_array(expecting_dimension);

    return result;
}

Array IREEBufferView::estimate_shape(const PackedByteArray& p_value) {
    Array result;
    result.append(p_value.size());
    return result;
}

Array IREEBufferView::estimate_shape(const PackedInt32Array& p_value) {
    Array result;
    result.append(p_value.size());
    return result;
}
Array IREEBufferView::estimate_shape(const PackedInt64Array& p_value) {
    Array result;
    result.append(p_value.size());
    return result;
}

Array IREEBufferView::estimate_shape(const PackedFloat32Array& p_value) {
    Array result;
    result.append(p_value.size());
    return result;
}

Array IREEBufferView::estimate_shape(const PackedFloat64Array& p_value) {
    Array result;
    result.append(p_value.size());
    return result;
}

Array IREEBufferView::estimate_shape(const PackedVector2Array& p_value) {
    Array result;
    result.append(p_value.size());
    result.append(2);
    return result;
}

Array IREEBufferView::estimate_shape(const PackedVector3Array& p_value) {
    Array result;
    result.append(p_value.size());
    result.append(3);
    return result;
}

Array IREEBufferView::estimate_shape(const PackedColorArray& p_value) {
    Array result;
    result.append(p_value.size());
    result.append(4);
    return result;
}

Array IREEBufferView::estimate_shape(const Vector2& p_value) {
    Array result;
    result.append(2);
    return result;
}

Array IREEBufferView::estimate_shape(const Vector2i& p_value) {
    Array result;
    result.append(2);
    return result;
}

Array IREEBufferView::estimate_shape(const Vector3& p_value) {
    Array result;
    result.append(3);
    return result;
}

Array IREEBufferView::estimate_shape(const Vector3i& p_value) {
    Array result;
    result.append(3);
    return result;
}

Array IREEBufferView::estimate_shape(const Vector4& p_value) {
    Array result;
    result.append(4);
    return result;
}

Array IREEBufferView::estimate_shape(const Vector4i& p_value) {
    Array result;
    result.append(4);
    return result;
}

Array IREEBufferView::estimate_shape(const Color& p_value) {
    Array result;
    result.append(4);
    return result;
}

Array IREEBufferView::estimate_shape(const Object* p_value) {
    if(p_value == nullptr) return Array();
    String class_name = p_value->get_class();
    if(class_name == "Image") return estimate_shape(Ref<Image>(p_value));
    else return Array();
}

Array IREEBufferView::estimate_shape(const Ref<Image> p_value) {
    if(p_value == nullptr) return Array();
    const iree_host_size_t channel_count = get_image_format_channel_count(p_value->get_format());
    if(channel_count == 0) return Array();
    Array result;
    result.append(p_value->get_width());
    result.append(p_value->get_height());
    result.append(channel_count);
    return result;
}

Ref<IREEBufferView> IREEBufferView::from(const Variant& p_value, ElementType p_value_type) {
    Ref<IREEBufferView> buffer_view_ref;
    buffer_view_ref.instantiate();

    iree_hal_buffer_view_t* raw_buffer_view = nullptr;
    Array shape_array = estimate_shape(p_value);    
    const int64_t shape_rank = shape_array.size();
    ERR_FAIL_COND_V_MSG(shape_rank == 0, Ref<IREEBufferView>(), "Unable to convert dimensionless value to IREE buffer view.");
    ERR_FAIL_COND_V_MSG(shape_rank > MAX_SHAPE_RANK, Ref<IREEBufferView>(), "Unable to convert such high dimension value.");

    iree_hal_dim_t shape[MAX_SHAPE_RANK] = {0};
    for(int i = 0; i < shape_rank; i++) shape[i] = shape_array[i];

    RawByteArray bytes;
    ERR_FAIL_COND_V(bytes.append(p_value, p_value_type), Ref<IREEBufferView>());

    ERR_FAIL_COND_V_MSG(
        iree_hal_buffer_view_allocate_buffer(
            iree_hal_device_allocator(IREE::get_hal_device()), shape_rank, shape,
            p_value_type, IREE_HAL_ENCODING_TYPE_DENSE_ROW_MAJOR,
            (iree_hal_buffer_params_t){
                .usage = IREE_HAL_BUFFER_USAGE_DEFAULT,
                .type = IREE_HAL_MEMORY_TYPE_DEVICE_LOCAL
            },
            iree_make_const_byte_span(bytes.get_data(), bytes.get_length()), &raw_buffer_view
    ), Ref<IREEBufferView>(), "Unable to allocate buffer for IREE buffer view.");

    buffer_view_ref->set_move_raw_buffer_view(raw_buffer_view);
    return buffer_view_ref;
}

bool IREEBufferView::is_null() const {
    return buffer_view == nullptr;
}

Array IREEBufferView::to_array() const {
    if(buffer_view == nullptr) return Array();
    PackedByteArray data = extract_bytes();
    if(data.size() == 0) return Array();

    Array result;
    const iree_hal_element_type_t element_type = iree_hal_buffer_view_element_type(buffer_view);

    switch(element_type) {
        case IREE_HAL_ELEMENT_TYPE_NONE:
        case IREE_HAL_ELEMENT_TYPE_OPAQUE_8:
        case IREE_HAL_ELEMENT_TYPE_OPAQUE_16:
        case IREE_HAL_ELEMENT_TYPE_BOOL_8:
        case IREE_HAL_ELEMENT_TYPE_INT_4:            
        case IREE_HAL_ELEMENT_TYPE_SINT_4:           
        case IREE_HAL_ELEMENT_TYPE_UINT_4:           
        case IREE_HAL_ELEMENT_TYPE_INT_8:
        case IREE_HAL_ELEMENT_TYPE_SINT_8:           
        case IREE_HAL_ELEMENT_TYPE_UINT_8:           
        case IREE_HAL_ELEMENT_TYPE_INT_16:
        case IREE_HAL_ELEMENT_TYPE_SINT_16:
        case IREE_HAL_ELEMENT_TYPE_UINT_16:
        case IREE_HAL_ELEMENT_TYPE_FLOAT_16:
        case IREE_HAL_ELEMENT_TYPE_BFLOAT_16:
        case IREE_HAL_ELEMENT_TYPE_COMPLEX_FLOAT_64: 
        case IREE_HAL_ELEMENT_TYPE_COMPLEX_FLOAT_128:
        default:
            ERR_PRINT("Unable to convert value.");
            return Array();

        case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
        case IREE_HAL_ELEMENT_TYPE_INT_32:
        case IREE_HAL_ELEMENT_TYPE_SINT_32:
        case IREE_HAL_ELEMENT_TYPE_UINT_32: {
            result = data.to_int32_array();
            break;
        }

        case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
        case IREE_HAL_ELEMENT_TYPE_INT_64:
        case IREE_HAL_ELEMENT_TYPE_SINT_64:
        case IREE_HAL_ELEMENT_TYPE_UINT_64: {
            result = data.to_int64_array();
            break;
        }

        case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
            result = data.to_float32_array();
            break;
        }

        case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
            result = data.to_float64_array();
            break;
        }
    }

    const iree_hal_dim_t* dimension = iree_hal_buffer_view_shape_dims(buffer_view);
    iree_host_size_t shape_rank = iree_hal_buffer_view_shape_rank(buffer_view);
    for(int64_t i = shape_rank - 1; i > 0; i--) {
        const iree_hal_dim_t elements_per_group = dimension[i];
        const uint64_t element_count = result.size();
        ERR_FAIL_COND_V_MSG(
            element_count % elements_per_group != 0 || elements_per_group > element_count, 
            Array(), "Unable to group elements by shape of buffer view, data corrupted(?)."
        );

        const uint64_t result_element_count = element_count / elements_per_group;
        for(uint64_t i = 0; i < result_element_count; i++)
            result.append(result.slice(
                i * elements_per_group, (i + 1) * elements_per_group
            ));

    }

    ERR_FAIL_COND_V_MSG(
        result.size() != dimension[0], 
        Array(), "Unable to group elements by shape of buffer view, data corrupted(?)."
    );

    return result;
}

Ref<Image> IREEBufferView::to_image(Image::Format p_format) const {
    if(buffer_view == nullptr) return Ref<Image>();
    iree_host_size_t channel_count = get_image_format_channel_count(p_format);
    ERR_FAIL_COND_V_MSG(channel_count == 0, Ref<Image>(), "Unsupported image format.");

    const iree_hal_dim_t* dimensions = iree_hal_buffer_view_shape_dims(buffer_view);
    iree_host_size_t shape_rank = iree_hal_buffer_view_shape_rank(buffer_view);

    ERR_FAIL_COND_V_MSG(shape_rank != 3, Ref<Image>(), "Unable to convert IREE buffer view with shape not equal to 3 to image.");
    ERR_FAIL_COND_V_MSG(dimensions[2] != channel_count, Ref<Image>(), "Incorrect channel count.");

    PackedByteArray data = extract_bytes();
    if(data.size() == 0) return Ref<Image>();

    return Image::create_from_data(dimensions[0], dimensions[1], false, p_format, data);
}

PackedByteArray IREEBufferView::extract_bytes() const {
    if(buffer_view == nullptr) return PackedByteArray();
    PackedByteArray data;

    iree_device_size_t data_size = iree_hal_buffer_view_byte_length(buffer_view);
    data.resize(data_size);
    ERR_FAIL_COND_V_MSG(data.size() != data_size, PackedByteArray(), "Fail to allocate memory for Packed Byte Array.");

    ERR_FAIL_COND_V_MSG(
        iree_hal_device_transfer_d2h(
            IREE::get_hal_device(), iree_hal_buffer_view_buffer(buffer_view),
            0, data.ptrw(), data_size, IREE_HAL_TRANSFER_BUFFER_FLAG_DEFAULT, iree_infinite_timeout()
    ), PackedByteArray(), "Unable to retrieve data from device.");

    return data;
}

iree_hal_buffer_view_t* IREEBufferView::get_retain_raw_buffer_view() {
    if(buffer_view == nullptr) return nullptr;
    iree_hal_buffer_view_retain(buffer_view);
    return buffer_view;
}

iree_hal_buffer_view_t* IREEBufferView::get_assign_raw_buffer_view() {
    if(buffer_view == nullptr) return nullptr;
    return buffer_view;
}

void IREEBufferView::set_move_raw_buffer_view(iree_hal_buffer_view_t* p_buffer_view) {
    if(buffer_view != nullptr) iree_hal_buffer_view_release(buffer_view);
    buffer_view = p_buffer_view;
}

void IREEBufferView::clean() {
    if(buffer_view != nullptr) {
        iree_hal_buffer_view_release(buffer_view);
        buffer_view = nullptr;
    }
}

IREEBufferView::IREEBufferView()
:
    buffer_view(nullptr)
{}

IREEBufferView::IREEBufferView(IREEBufferView&& m_buffer_view)
:
    buffer_view(m_buffer_view.buffer_view)
{
    m_buffer_view.buffer_view = nullptr;
}

IREEBufferView::~IREEBufferView() { clean(); }