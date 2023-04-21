#include "iree_data.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include "iree.h"

#define MAX_SHAPE_RANK 20

using namespace godot;

#define IS_NUMERIC_ARRAY_LIKE(mp_value_type) ( \
    ((mp_value_type) == Variant::Type::ARRAY) || \
    ((mp_value_type) == Variant::Type::PACKED_BYTE_ARRAY) || \
    ((mp_value_type) == Variant::Type::PACKED_INT32_ARRAY) || \
    ((mp_value_type) == Variant::Type::PACKED_INT64_ARRAY) || \
    ((mp_value_type) == Variant::Type::PACKED_FLOAT32_ARRAY) || \
    ((mp_value_type) == Variant::Type::PACKED_FLOAT64_ARRAY) || \
    ((mp_value_type) == Variant::Type::PACKED_VECTOR2_ARRAY) || \
    ((mp_value_type) == Variant::Type::PACKED_VECTOR3_ARRAY) || \
    ((mp_value_type) == Variant::Type::PACKED_COLOR_ARRAY) || \
    ((mp_value_type) == Variant::Type::VECTOR2) || \
    ((mp_value_type) == Variant::Type::VECTOR2I) || \
    ((mp_value_type) == Variant::Type::VECTOR3) || \
    ((mp_value_type) == Variant::Type::VECTOR3I) || \
    ((mp_value_type) == Variant::Type::VECTOR4) || \
    ((mp_value_type) == Variant::Type::VECTOR4I) || \
    ((mp_value_type) == Variant::Type::COLOR) \
)

void IREEData::_bind_methods() {
}

Array IREEData::estimate_dimension(const Variant& p_value) {
    switch(p_value.get_type()) {
        case Variant::Type::ARRAY: return estimate_dimension(Array(p_value));
        case Variant::Type::PACKED_BYTE_ARRAY: return estimate_dimension(PackedByteArray(p_value));
        case Variant::Type::PACKED_INT32_ARRAY: return estimate_dimension(PackedInt32Array(p_value));
        case Variant::Type::PACKED_INT64_ARRAY: return estimate_dimension(PackedInt64Array(p_value));
        case Variant::Type::PACKED_FLOAT32_ARRAY: return estimate_dimension(PackedFloat32Array(p_value));
        case Variant::Type::PACKED_FLOAT64_ARRAY: return estimate_dimension(PackedFloat64Array(p_value));
        case Variant::Type::PACKED_VECTOR2_ARRAY: return estimate_dimension(PackedVector2Array(p_value));
        case Variant::Type::PACKED_VECTOR3_ARRAY: return estimate_dimension(PackedVector3Array(p_value));
        case Variant::Type::PACKED_COLOR_ARRAY: return estimate_dimension(PackedColorArray(p_value));
        case Variant::Type::VECTOR2: return estimate_dimension(Vector2(p_value));
        case Variant::Type::VECTOR2I: return estimate_dimension(Vector2i(p_value));
        case Variant::Type::VECTOR3: return estimate_dimension(Vector3(p_value));
        case Variant::Type::VECTOR3I: return estimate_dimension(Vector3i(p_value));
        case Variant::Type::COLOR: return estimate_dimension(Color(p_value));
        case Variant::Type::VECTOR4: return estimate_dimension(Vector4(p_value));
        case Variant::Type::VECTOR4I: return estimate_dimension(Vector4i(p_value));
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
        case Variant::Type::OBJECT:
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

Array IREEData::estimate_dimension(const Array& p_value) {
    Array result;

    bool expecting_atomic = true;
    Array expecting_dimension; // 0 -> not expecting dimensional variant; else -> enforce the dimensional element has the same dimension.
    int64_t size = p_value.size();
    for(int64_t i = 0; i < size; i++) {
        const Variant& element = p_value[i];
        const Variant::Type element_type = element.get_type();
        bool is_numeric_array_like = IS_NUMERIC_ARRAY_LIKE(element_type);

        if(i == 0) 
            if(is_numeric_array_like) {
                expecting_atomic = false;
                expecting_dimension = estimate_dimension(element);
                if(expecting_dimension.size() == 0) return Array();
                continue;
            }

        // Expecting atomic.
        if(expecting_atomic) {
            if(element_type != Variant::Type::INT && element_type != Variant::Type::FLOAT) 
                return Array();
        }

        // Expecting array.
        else if(estimate_dimension(element) != expecting_dimension) return Array();
    }       

    result.append(size);
    result.append_array(expecting_dimension);

    return result;
}

Array IREEData::estimate_dimension(const PackedByteArray& p_value) {
    Array result;
    result.append(p_value.size());
    return result;
}

Array IREEData::estimate_dimension(const PackedInt32Array& p_value) {
    Array result;
    result.append(p_value.size());
    return result;
}
Array IREEData::estimate_dimension(const PackedInt64Array& p_value) {
    Array result;
    result.append(p_value.size());
    return result;
}

Array IREEData::estimate_dimension(const PackedFloat32Array& p_value) {
    Array result;
    result.append(p_value.size());
    return result;
}

Array IREEData::estimate_dimension(const PackedFloat64Array& p_value) {
    Array result;
    result.append(p_value.size());
    return result;
}

Array IREEData::estimate_dimension(const PackedVector2Array& p_value) {
    Array result;
    result.append(p_value.size());
    result.append(2);
    return result;
}

Array IREEData::estimate_dimension(const PackedVector3Array& p_value) {
    Array result;
    result.append(p_value.size());
    result.append(3);
    return result;
}

Array IREEData::estimate_dimension(const PackedColorArray& p_value) {
    Array result;
    result.append(p_value.size());
    result.append(4);
    return result;
}

Array IREEData::estimate_dimension(const Vector2& p_value) {
    Array result;
    result.append(2);
    return result;
}

Array IREEData::estimate_dimension(const Vector2i& p_value) {
    Array result;
    result.append(2);
    return result;
}

Array IREEData::estimate_dimension(const Vector3& p_value) {
    Array result;
    result.append(3);
    return result;
}

Array IREEData::estimate_dimension(const Vector3i& p_value) {
    Array result;
    result.append(3);
    return result;
}

Array IREEData::estimate_dimension(const Vector4& p_value) {
    Array result;
    result.append(4);
    return result;
}

Array IREEData::estimate_dimension(const Vector4i& p_value) {
    Array result;
    result.append(4);
    return result;
}

Array IREEData::estimate_dimension(const Color& p_value) {
    Array result;
    result.append(4);
    return result;
}

iree_hal_buffer_view_t* IREEData::create_raw_buffer_view_from_value(const Variant& p_value, iree_hal_element_type_t p_value_type) {
    iree_hal_buffer_view_t* buffer_view = nullptr;
    Array dimension = estimate_dimension(p_value);    
    const int64_t shape_rank = dimension.size();
    ERR_FAIL_COND_V_MSG(shape_rank == 0, nullptr, "Unable to convert dimensionless value to IREE buffer view.");
    ERR_FAIL_COND_V_MSG(shape_rank > MAX_SHAPE_RANK, nullptr, "Unable to convert such high dimension value.");

    iree_hal_dim_t shape[MAX_SHAPE_RANK] = {0};
    for(int i = 0; i < shape_rank; i++) shape[i] = dimension[i];

    RawByteArray bytes;
    ERR_FAIL_COND_V(push_value_into_byte_array(p_value, p_value_type, bytes), nullptr);

    ERR_FAIL_COND_V_MSG(
        iree_hal_buffer_view_allocate_buffer(
            iree_hal_device_allocator(IREE::get_hal_device()), shape_rank, shape,
            p_value_type, IREE_HAL_ENCODING_TYPE_DENSE_ROW_MAJOR,
            (iree_hal_buffer_params_t){
                .usage = IREE_HAL_BUFFER_USAGE_DEFAULT,
                .type = IREE_HAL_MEMORY_TYPE_DEVICE_LOCAL
            },
            iree_make_const_byte_span(bytes.get_data(), bytes.get_length()), &buffer_view
    ), nullptr, "Unable to allocate buffer for IREE buffer view.");

    return buffer_view;
}

Error IREEData::push_value_into_byte_array(const Variant& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    switch(p_value.get_type()) {
        case Variant::Type::ARRAY: return push_value_into_byte_array(Array(p_value), p_value_type, m_bytes);
        case Variant::Type::PACKED_BYTE_ARRAY: return push_value_into_byte_array(PackedByteArray(p_value), p_value_type, m_bytes);
        case Variant::Type::PACKED_INT32_ARRAY: return push_value_into_byte_array(PackedInt32Array(p_value), p_value_type, m_bytes);
        case Variant::Type::PACKED_INT64_ARRAY: return push_value_into_byte_array(PackedInt64Array(p_value), p_value_type, m_bytes);
        case Variant::Type::PACKED_FLOAT32_ARRAY: return push_value_into_byte_array(PackedFloat32Array(p_value), p_value_type, m_bytes);
        case Variant::Type::PACKED_FLOAT64_ARRAY: return push_value_into_byte_array(PackedFloat64Array(p_value), p_value_type, m_bytes);
        case Variant::Type::PACKED_VECTOR2_ARRAY: return push_value_into_byte_array(PackedVector2Array(p_value), p_value_type, m_bytes);
        case Variant::Type::PACKED_VECTOR3_ARRAY: return push_value_into_byte_array(PackedVector3Array(p_value), p_value_type, m_bytes);
        case Variant::Type::PACKED_COLOR_ARRAY: return push_value_into_byte_array(PackedColorArray(p_value), p_value_type, m_bytes);
        case Variant::Type::VECTOR2: return push_value_into_byte_array(Vector2(p_value), p_value_type, m_bytes);
        case Variant::Type::VECTOR2I: return push_value_into_byte_array(Vector2i(p_value), p_value_type, m_bytes);
        case Variant::Type::VECTOR3: return push_value_into_byte_array(Vector3(p_value), p_value_type, m_bytes);
        case Variant::Type::VECTOR3I: return push_value_into_byte_array(Vector3i(p_value), p_value_type, m_bytes);
        case Variant::Type::COLOR: return push_value_into_byte_array(Color(p_value), p_value_type, m_bytes);
        case Variant::Type::VECTOR4: return push_value_into_byte_array(Vector4(p_value), p_value_type, m_bytes);
        case Variant::Type::VECTOR4I: return push_value_into_byte_array(Vector4i(p_value), p_value_type, m_bytes);
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
        case Variant::Type::OBJECT:
        case Variant::Type::CALLABLE:
        case Variant::Type::SIGNAL:
        case Variant::Type::DICTIONARY:
        case Variant::Type::PACKED_STRING_ARRAY:
        case Variant::Type::VARIANT_MAX:
        case Variant::Type::NIL:
        default:
            ERR_PRINT(vformat("Unable to convert variant with the type '%s' into IREE list.", Variant::get_type_name(p_value.get_type())));
            return FAILED;
    }
}

Error IREEData::push_value_into_byte_array(const Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    const int element_count = p_value.size();

    for(int i = 0; i < element_count; i++) {
        const Variant& element = p_value[i];
        const Variant::Type element_type = element.get_type();

        if(IS_NUMERIC_ARRAY_LIKE(element_type)) {
            Error err = push_value_into_byte_array(element, p_value_type, m_bytes);
            if(err != OK) return err;
        }
        else {
            if(element_type != Variant::Type::INT && element_type != Variant::Type::FLOAT) {
                ERR_PRINT(vformat("Non-numerical value (type '%s') in the array.", Variant::get_type_name(element_type)));
                goto on_fail;
            }

            switch(p_value_type) {
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
                    ERR_PRINT("Unable to convert value in array.");
                    goto on_fail;

                case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: {
                    if(element_type == Variant::Type::INT) m_bytes.append_bytes<int32_t>(element);
                    else {
#ifdef REAL_T_IS_DOUBLE
                        ERR_PRINT("Unable to convert value in array.");
                        goto on_fail;
#else
                        m_bytes.append_bytes<float>(element);
#endif
                    }
                    break;
                }

                case IREE_HAL_ELEMENT_TYPE_OPAQUE_64: {
                    if(element_type == Variant::Type::INT)  m_bytes.append_bytes<int64_t>(element);
                    else
#ifdef REAL_T_IS_DOUBLE
                        m_bytes.append_bytes(double(element));
#else
                        m_bytes.append_bytes<double>(float(element));
#endif
                    break;
                }

                case IREE_HAL_ELEMENT_TYPE_INT_32:
                case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                    if(element_type == Variant::Type::INT) m_bytes.append_bytes<int32_t>(element);
                    else {
#ifdef REAL_T_IS_DOUBLE
                        ERR_PRINT("Unable to convert value in array.");
                        goto on_fail;
#else
                        WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                        m_bytes.append_bytes<int32_t>(float(element));
#endif
                    }
                    break;
                }

                case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                    if(element_type == Variant::Type::INT) m_bytes.append_bytes<uint32_t>(int32_t(element));
                    else {
#ifdef REAL_T_IS_DOUBLE
                        ERR_PRINT("Unable to convert value in array.");
                        goto on_fail;
#else
                        WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                        m_bytes.append_bytes<uint32_t>(float(element));
#endif
                    }
                    break;
                }

                case IREE_HAL_ELEMENT_TYPE_INT_64:
                case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                    if(element_type == Variant::Type::INT) m_bytes.append_bytes<int64_t> (element);
                    else {
                        WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
#ifdef REAL_T_IS_DOUBLE
                        m_bytes.append_bytes<int64_t>(double(element));
#else
                        m_bytes.append_bytes<int64_t>(float(element));
#endif
                    }
                    break;
                }

                case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                    if(element_type == Variant::Type::INT) m_bytes.append_bytes<uint64_t> (int64_t(element));
                    else {
                        WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
#ifdef REAL_T_IS_DOUBLE
                        m_bytes.append_bytes<uint64_t>(double(element));
#else
                        m_bytes.append_bytes<uint64_t>(float(element));
#endif
                    }
                    break;
                }

                case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                    if(element_type == Variant::Type::INT) m_bytes.append_bytes<float>(int32_t(element));
                    else {
#ifdef REAL_T_IS_DOUBLE
                        ERR_PRINT("Unable to convert value in array.");
                        goto on_fail;
#else
                        m_bytes.append_bytes<float>(element);
#endif
                    }
                    break;
                }

                case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                    if(element_type == Variant::Type::INT) m_bytes.append_bytes<double>(int64_t(element));
                    else
#ifdef REAL_T_IS_DOUBLE
                        m_bytes.append_bytes<double>(element);
#else
                        m_bytes.append_bytes<float>(element);
#endif
                    break;
                }
            }

        }
    }

    return OK;

on_fail:
    m_bytes.clear();
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const PackedByteArray& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const int8_t byte : p_value)
        switch(p_value_type) {
            case IREE_HAL_ELEMENT_TYPE_NONE:
            case IREE_HAL_ELEMENT_TYPE_INT_4:            
            case IREE_HAL_ELEMENT_TYPE_SINT_4:           
            case IREE_HAL_ELEMENT_TYPE_UINT_4:           
            case IREE_HAL_ELEMENT_TYPE_FLOAT_16:
            case IREE_HAL_ELEMENT_TYPE_BFLOAT_16:
            case IREE_HAL_ELEMENT_TYPE_COMPLEX_FLOAT_64: 
            case IREE_HAL_ELEMENT_TYPE_COMPLEX_FLOAT_128:
            default:
                ERR_PRINT("Unable to convert value in array.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_8:
            case IREE_HAL_ELEMENT_TYPE_BOOL_8:
            case IREE_HAL_ELEMENT_TYPE_INT_8:
            case IREE_HAL_ELEMENT_TYPE_SINT_8: {
                m_bytes.append_bytes<int8_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_8: {
                m_bytes.append_bytes<uint8_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_16:
            case IREE_HAL_ELEMENT_TYPE_INT_16:
            case IREE_HAL_ELEMENT_TYPE_SINT_16: {
                m_bytes.append_bytes<uint16_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_16: {
                m_bytes.append_bytes<uint16_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                m_bytes.append_bytes<int32_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                m_bytes.append_bytes<int64_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                m_bytes.append_bytes<uint32_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                m_bytes.append_bytes<uint64_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                m_bytes.append_bytes<float>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                m_bytes.append_bytes<double>(byte);
                break;
            }
        }


    return OK;

on_fail:
    m_bytes.clear();
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const PackedInt32Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const int32_t number : p_value)
        switch(p_value_type) {
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
                ERR_PRINT("Unable to convert value in array.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                m_bytes.append_bytes<int32_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                m_bytes.append_bytes<int64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                m_bytes.append_bytes<uint32_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                m_bytes.append_bytes<uint64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                m_bytes.append_bytes<float>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                m_bytes.append_bytes<double>(number);
                break;
            }
        }


    return OK;

on_fail:
    m_bytes.clear();
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const PackedInt64Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const int64_t number : p_value)
        switch(p_value_type) {
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
            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32:
            case IREE_HAL_ELEMENT_TYPE_UINT_32:
            case IREE_HAL_ELEMENT_TYPE_FLOAT_32:
            default:
                ERR_PRINT("Unable to convert value in array.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                m_bytes.append_bytes<int64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                m_bytes.append_bytes<uint64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                m_bytes.append_bytes<double>(number);
                break;
            }
        }


    return OK;

on_fail:
    m_bytes.clear();
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const PackedFloat32Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const float number : p_value)
        switch(p_value_type) {
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
                ERR_PRINT("Unable to convert value in array.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                m_bytes.append_bytes<int32_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                m_bytes.append_bytes<int64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                m_bytes.append_bytes<uint32_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                m_bytes.append_bytes<uint64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                m_bytes.append_bytes<float>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                m_bytes.append_bytes<double>(number);
                break;
            }
        }

    return OK;

on_fail:
    m_bytes.clear();
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const PackedFloat64Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const float number : p_value)
        switch(p_value_type) {
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
            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32:
            case IREE_HAL_ELEMENT_TYPE_UINT_32:
            case IREE_HAL_ELEMENT_TYPE_FLOAT_32:
            default:
                ERR_PRINT("Unable to convert value in array.");
                goto on_fail;


            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                m_bytes.append_bytes<int64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                m_bytes.append_bytes<uint64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                m_bytes.append_bytes<double>(number);
                break;
            }
        }

    return OK;

on_fail:
    m_bytes.clear();
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const PackedVector2Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const Vector2& vector : p_value) {
        Error err = push_value_into_byte_array(vector, p_value_type, m_bytes);
        if(err) return err;
    }

    return OK;
}

Error IREEData::push_value_into_byte_array(const PackedVector3Array& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const Vector3& vector : p_value) {
        Error err = push_value_into_byte_array(vector, p_value_type, m_bytes);
        if(err) return err;
    }

    return OK;
}

Error IREEData::push_value_into_byte_array(const PackedColorArray& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const Color& color : p_value) {
        Error err = push_value_into_byte_array(color, p_value_type, m_bytes);
        if(err) return err;
    }

    return OK;
}

Error IREEData::push_value_into_byte_array(const Vector2& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_PRINT("Yet to be implemented");
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const Vector2i& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_PRINT("Yet to be implemented");
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const Vector3& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_PRINT("Yet to be implemented");
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const Vector3i& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_PRINT("Yet to be implemented");
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const Vector4& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_PRINT("Yet to be implemented");
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const Vector4i& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_PRINT("Yet to be implemented");
    return FAILED;
}

Error IREEData::push_value_into_byte_array(const Color& p_value, iree_hal_element_type_t p_value_type, RawByteArray& m_bytes) {
    ERR_PRINT("Yet to be implemented");
    return FAILED;
}

Array IREEData::create_array_from_raw_buffer_view(const iree_hal_buffer_view_t* p_buffer_view) {
    PackedByteArray data = create_bytes_from_raw_buffer_view(p_buffer_view);
    if(data.size() == 0) return Array();

    Array result;
    const iree_hal_element_type_t element_type = iree_hal_buffer_view_element_type(p_buffer_view);

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
            ERR_PRINT("Unable to convert value in array.");
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

    const iree_hal_dim_t* dimension = iree_hal_buffer_view_shape_dims(p_buffer_view);
    iree_host_size_t shape_rank = iree_hal_buffer_view_shape_rank(p_buffer_view);
    for(int64_t i = shape_rank - 1; i >= 0; i--) {
        const iree_hal_dim_t element_count = dimension[i];
        result = group_elements(result, element_count);
    }

    return result;
}

PackedByteArray IREEData::create_bytes_from_raw_buffer_view(const iree_hal_buffer_view_t* p_buffer_view) {
    PackedByteArray data;

    iree_device_size_t data_size = iree_hal_buffer_view_byte_length(p_buffer_view);
    data.resize(data_size);
    ERR_FAIL_COND_V_MSG(data.size() != data_size, PackedByteArray(), "Fail to allocate memory for Packed Byte Array.");

    ERR_FAIL_COND_V_MSG(
        iree_hal_device_transfer_d2h(
            IREE::get_hal_device(), iree_hal_buffer_view_buffer(p_buffer_view),
            0, data.ptrw(), data_size, IREE_HAL_TRANSFER_BUFFER_FLAG_DEFAULT, iree_infinite_timeout()
    ), PackedByteArray(), "Unable to retrieve data from device.");

    return data;
}

Array IREEData::group_elements(const Array& p_array, uint64_t p_elements_per_group) {
    const uint64_t element_count = p_array.size();
    ERR_FAIL_COND_V_MSG(
        element_count % p_elements_per_group != 0 || p_elements_per_group > element_count, 
        Array(), vformat("Unable to group elements by %u.", p_elements_per_group)
    );

    Array result;
    const uint64_t result_element_count = element_count / p_elements_per_group;
    for(uint64_t i = 0; i < result_element_count; i++)
        result.append(p_array.slice(
            i * p_elements_per_group, (i + 1) * p_elements_per_group
        ));

    return result;
}

IREEData::IREEData() {}

IREEData::~IREEData() { }