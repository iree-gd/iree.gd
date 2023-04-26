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

Error RawByteArray::append(const Variant& p_value, iree_hal_element_type_t p_value_type) {
    switch(p_value.get_type()) {
        case Variant::Type::ARRAY: return append(Array(p_value), p_value_type);
        case Variant::Type::PACKED_BYTE_ARRAY: return append(PackedByteArray(p_value), p_value_type);
        case Variant::Type::PACKED_INT32_ARRAY: return append(PackedInt32Array(p_value), p_value_type);
        case Variant::Type::PACKED_INT64_ARRAY: return append(PackedInt64Array(p_value), p_value_type);
        case Variant::Type::PACKED_FLOAT32_ARRAY: return append(PackedFloat32Array(p_value), p_value_type);
        case Variant::Type::PACKED_FLOAT64_ARRAY: return append(PackedFloat64Array(p_value), p_value_type);
        case Variant::Type::PACKED_VECTOR2_ARRAY: return append(PackedVector2Array(p_value), p_value_type);
        case Variant::Type::PACKED_VECTOR3_ARRAY: return append(PackedVector3Array(p_value), p_value_type);
        case Variant::Type::PACKED_COLOR_ARRAY: return append(PackedColorArray(p_value), p_value_type);
        case Variant::Type::VECTOR2: return append(Vector2(p_value), p_value_type);
        case Variant::Type::VECTOR2I: return append(Vector2i(p_value), p_value_type);
        case Variant::Type::VECTOR3: return append(Vector3(p_value), p_value_type);
        case Variant::Type::VECTOR3I: return append(Vector3i(p_value), p_value_type);
        case Variant::Type::COLOR: return append(Color(p_value), p_value_type);
        case Variant::Type::VECTOR4: return append(Vector4(p_value), p_value_type);
        case Variant::Type::VECTOR4I: return append(Vector4i(p_value), p_value_type);
        case Variant::Type::OBJECT: return append((Object*)(p_value), p_value_type);
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
            ERR_PRINT(vformat("Unable to convert variant with the type '%s' into IREE list.", Variant::get_type_name(p_value.get_type())));
            return FAILED;
    }
}

Error RawByteArray::append(const Array& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    const int element_count = p_value.size();

    for(int i = 0; i < element_count; i++) {
        const Variant& element = p_value[i];
        const Variant::Type element_type = element.get_type();

        // Primitive types.
        if(element_type == Variant::Type::INT || element_type == Variant::Type::FLOAT)
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: {
                if(element_type == Variant::Type::INT) append<int32_t>(element);
                else {
#ifdef REAL_T_IS_DOUBLE
                    ERR_PRINT("Unable to convert value.");
                    goto on_fail;
#else
                    append<float>(element);
#endif
                }
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64: {
                if(element_type == Variant::Type::INT)  append<int64_t>(element);
                else
#ifdef REAL_T_IS_DOUBLE
                    append(double(element));
#else
                    append<double>(float(element));
#endif
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                if(element_type == Variant::Type::INT) append<int32_t>(element);
                else {
#ifdef REAL_T_IS_DOUBLE
                    ERR_PRINT("Unable to convert value.");
                    goto on_fail;
#else
                    WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                    append<int32_t>(float(element));
#endif
                }
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                if(element_type == Variant::Type::INT) append<uint32_t>(int32_t(element));
                else {
#ifdef REAL_T_IS_DOUBLE
                    ERR_PRINT("Unable to convert value.");
                    goto on_fail;
#else
                    WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                    append<uint32_t>(float(element));
#endif
                }
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                if(element_type == Variant::Type::INT) append<int64_t> (element);
                else {
                    WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
#ifdef REAL_T_IS_DOUBLE
                    append<int64_t>(double(element));
#else
                    append<int64_t>(float(element));
#endif
                }
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                if(element_type == Variant::Type::INT) append<uint64_t> (int64_t(element));
                else {
                    WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
#ifdef REAL_T_IS_DOUBLE
                    append<uint64_t>(double(element));
#else
                    append<uint64_t>(float(element));
#endif
                }
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                if(element_type == Variant::Type::INT) append<float>(int32_t(element));
                else {
#ifdef REAL_T_IS_DOUBLE
                    ERR_PRINT("Unable to convert value.");
                    goto on_fail;
#else
                    append<float>(element);
#endif
                }
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                if(element_type == Variant::Type::INT) append<double>(int64_t(element));
                else
#ifdef REAL_T_IS_DOUBLE
                    append<double>(element);
#else
                    append<float>(element);
#endif
                break;
            }
        }

        else {
            Error err = append(element, p_value_type);
            if(err != OK) return err;
        }
    }

    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const PackedByteArray& p_value, iree_hal_element_type_t p_value_type) {
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_8:
            case IREE_HAL_ELEMENT_TYPE_BOOL_8:
            case IREE_HAL_ELEMENT_TYPE_INT_8:
            case IREE_HAL_ELEMENT_TYPE_SINT_8: {
                append<int8_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_8: {
                append<uint8_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_16:
            case IREE_HAL_ELEMENT_TYPE_INT_16:
            case IREE_HAL_ELEMENT_TYPE_SINT_16: {
                append<uint16_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_16: {
                append<uint16_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                append<int32_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                append<int64_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                append<uint32_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                append<uint64_t>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                append<float>(byte);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(byte);
                break;
            }
        }


    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const PackedInt32Array& p_value, iree_hal_element_type_t p_value_type) {
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                append<int32_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                append<int64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                append<uint32_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                append<uint64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                append<float>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(number);
                break;
            }
        }


    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const PackedInt64Array& p_value, iree_hal_element_type_t p_value_type) {
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                append<int64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                append<uint64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(number);
                break;
            }
        }


    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const PackedFloat32Array& p_value, iree_hal_element_type_t p_value_type) {
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int32_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint32_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                append<float>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(number);
                break;
            }
        }

    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const PackedFloat64Array& p_value, iree_hal_element_type_t p_value_type) {
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;


            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint64_t>(number);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(number);
                break;
            }
        }

    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const PackedVector2Array& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const Vector2& vector : p_value) {
        Error err = append(vector, p_value_type);
        if(err) return err;
    }

    return OK;
}

Error RawByteArray::append(const PackedVector3Array& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const Vector3& vector : p_value) {
        Error err = append(vector, p_value_type);
        if(err) return err;
    }

    return OK;
}

Error RawByteArray::append(const PackedColorArray& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(const Color& color : p_value) {
        Error err = append(color, p_value_type);
        if(err) return err;
    }

    return OK;
}

Error RawByteArray::append(const Vector2& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(int i = 0; i < p_value.AXIS_COUNT; i++)
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int32_t>(p_value.coord[i]);
                break;
#endif
            }


            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint32_t>(p_value.coord[i]);
                break;
#endif
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int64_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint64_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                append<float>(p_value.coord[i]);
                break;
#endif
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(p_value.coord[i]);
                break;
            }
        }

    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const Vector2i& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(int i = 0; i < p_value.AXIS_COUNT; i++)
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                append<int32_t>(p_value.coord[i]);
                break;
            }


            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                append<uint32_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                append<int64_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                append<uint64_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                append<float>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(p_value.coord[i]);
                break;
            }
        }

    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const Vector3& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(int i = 0; i < p_value.AXIS_COUNT; i++)
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int32_t>(p_value.coord[i]);
                break;
#endif
            }


            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint32_t>(p_value.coord[i]);
                break;
#endif
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int64_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint64_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                append<float>(p_value.coord[i]);
                break;
#endif
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(p_value.coord[i]);
                break;
            }
        }

    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const Vector3i& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(int i = 0; i < p_value.AXIS_COUNT; i++)
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                append<int32_t>(p_value.coord[i]);
                break;
            }


            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                append<uint32_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                append<int64_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                append<uint64_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                append<float>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(p_value.coord[i]);
                break;
            }
        }

    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const Vector4& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(int i = 0; i < p_value.AXIS_COUNT; i++)
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int32_t>(p_value.components[i]);
                break;
#endif
            }


            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint32_t>(p_value.components[i]);
                break;
#endif
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int64_t>(p_value.components[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint64_t>(p_value.components[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                append<float>(p_value.components[i]);
                break;
#endif
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(p_value.components[i]);
                break;
            }
        }

    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const Vector4i& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(int i = 0; i < p_value.AXIS_COUNT; i++)
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
                append<int32_t>(p_value.coord[i]);
                break;
            }


            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
                append<uint32_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                append<int64_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                append<uint64_t>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
                append<float>(p_value.coord[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(p_value.coord[i]);
                break;
            }
        }

    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const Color& p_value, iree_hal_element_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_HAL_ELEMENT_TYPE_NONE, FAILED, "Unable to produce IREE list with undefined type.");

    for(int i = 0; i < 4; i++)
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
                ERR_PRINT("Unable to convert value.");
                goto on_fail;

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_32: 
            case IREE_HAL_ELEMENT_TYPE_INT_32:
            case IREE_HAL_ELEMENT_TYPE_SINT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int32_t>(p_value.components[i]);
                break;
#endif
            }


            case IREE_HAL_ELEMENT_TYPE_UINT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint32_t>(p_value.components[i]);
                break;
#endif
            }

            case IREE_HAL_ELEMENT_TYPE_OPAQUE_64:
            case IREE_HAL_ELEMENT_TYPE_INT_64:
            case IREE_HAL_ELEMENT_TYPE_SINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<int64_t>(p_value.components[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_UINT_64: {
                WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                append<uint64_t>(p_value.components[i]);
                break;
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_32: {
#ifdef REAL_T_IS_DOUBLE
                ERR_PRINT("Unable to convert value.");
                goto on_fail;
#else
                append<float>(p_value.components[i]);
                break;
#endif
            }

            case IREE_HAL_ELEMENT_TYPE_FLOAT_64: {
                append<double>(p_value.components[i]);
                break;
            }
        }

    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const Object* p_value, iree_hal_element_type_t p_value_type) {
    if(p_value == nullptr) return ERR_INVALID_PARAMETER;
    if(p_value->get_class() == "Image") return append(Ref<Image>(p_value), p_value_type);

    else {
        ERR_PRINT("Unable to convert value.");
        goto on_fail;
    }
    
    return OK;

on_fail:
    clear();
    return FAILED;
}

Error RawByteArray::append(const Ref<Image> p_value, iree_hal_element_type_t p_value_type) {
    if(p_value == nullptr) return ERR_INVALID_PARAMETER;
    const PackedByteArray data = p_value->get_data();
    append(data.ptr(), data.size());
    return OK;
}



RawByteArray::RawByteArray()
:
    data(nullptr),
    length(0),
    capacity(0)
{}

RawByteArray::~RawByteArray() { clean(); }