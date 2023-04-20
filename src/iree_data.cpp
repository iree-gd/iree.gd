#include "iree_data.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2i.hpp>

using namespace godot;

#define IS_NUMERIC_ARRAY_LIKE(mp_value_type) ( \
    ((mp_value_type) == Variant::Type::ARRAY) && \
    ((mp_value_type) == Variant::Type::PACKED_BYTE_ARRAY) && \
    ((mp_value_type) == Variant::Type::PACKED_INT32_ARRAY) && \
    ((mp_value_type) == Variant::Type::PACKED_INT64_ARRAY) && \
    ((mp_value_type) == Variant::Type::PACKED_FLOAT32_ARRAY) && \
    ((mp_value_type) == Variant::Type::PACKED_FLOAT64_ARRAY) && \
    ((mp_value_type) == Variant::Type::PACKED_VECTOR2_ARRAY) && \
    ((mp_value_type) == Variant::Type::PACKED_VECTOR3_ARRAY) && \
    ((mp_value_type) == Variant::Type::PACKED_COLOR_ARRAY) && \
    ((mp_value_type) == Variant::Type::VECTOR2) && \
    ((mp_value_type) == Variant::Type::VECTOR2I) && \
    ((mp_value_type) == Variant::Type::VECTOR3) && \
    ((mp_value_type) == Variant::Type::VECTOR3I) && \
    ((mp_value_type) == Variant::Type::VECTOR4) && \
    ((mp_value_type) == Variant::Type::VECTOR4I) && \
    ((mp_value_type) == Variant::Type::COLOR) \
)

IREEData::IREEData(iree_vm_list_t* p_data) 
:
    data(p_data)
{ }

void IREEData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("to_array"), &IREEData::to_array);
}

iree_vm_list_t* IREEData::value_to_raw_list(const Variant& p_value, iree_vm_value_type_t p_value_type) {
    switch(p_value.get_type()) {
        case Variant::Type::ARRAY: return value_to_raw_list(Array(p_value), p_value_type);
        case Variant::Type::PACKED_BYTE_ARRAY: return value_to_raw_list(PackedByteArray(p_value), p_value_type);
        case Variant::Type::PACKED_INT32_ARRAY: return value_to_raw_list(PackedInt32Array(p_value), p_value_type);
        case Variant::Type::PACKED_INT64_ARRAY: return value_to_raw_list(PackedInt64Array(p_value), p_value_type);
        case Variant::Type::PACKED_FLOAT32_ARRAY: return value_to_raw_list(PackedFloat32Array(p_value), p_value_type);
        case Variant::Type::PACKED_FLOAT64_ARRAY: return value_to_raw_list(PackedFloat64Array(p_value), p_value_type);
        case Variant::Type::PACKED_VECTOR2_ARRAY: return value_to_raw_list(PackedVector2Array(p_value), p_value_type);
        case Variant::Type::PACKED_VECTOR3_ARRAY: return value_to_raw_list(PackedVector3Array(p_value), p_value_type);
        case Variant::Type::PACKED_COLOR_ARRAY: return value_to_raw_list(PackedColorArray(p_value), p_value_type);
        case Variant::Type::VECTOR2: return value_to_raw_list(Vector2(p_value), p_value_type);
        case Variant::Type::VECTOR2I: return value_to_raw_list(Vector2i(p_value), p_value_type);
        case Variant::Type::VECTOR3: return value_to_raw_list(Vector3(p_value), p_value_type);
        case Variant::Type::VECTOR3I: return value_to_raw_list(Vector3i(p_value), p_value_type);
        case Variant::Type::COLOR: return value_to_raw_list(Color(p_value), p_value_type);
        case Variant::Type::VECTOR4: return value_to_raw_list(Vector4(p_value), p_value_type);
        case Variant::Type::VECTOR4I: return value_to_raw_list(Vector4i(p_value), p_value_type);
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
            return nullptr;
    }
}

iree_vm_list_t* IREEData::value_to_raw_list(const Array& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");

    iree_vm_list_t* list = nullptr;
    const int element_count = p_value.size();

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_undefined_type_def(), // Variant type.
            element_count, iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(int i = 0; i < element_count; i++) {
        const Variant& element = p_value[i];
        const Variant::Type element_type = element.get_type();

        if(IS_NUMERIC_ARRAY_LIKE(element_type)) {
            iree_vm_list_t* nested_list = value_to_raw_list(element, p_value_type);
            if(nested_list == nullptr) goto clean_up_list;
            iree_vm_variant_t retained = iree_vm_make_variant_ref_assign(iree_vm_list_retain_ref(nested_list));
            iree_vm_list_push_variant_move(list, &retained);
            iree_vm_list_release(nested_list);
        }
        else {
            iree_vm_variant_t variant;
            if(element_type != Variant::Type::INT || element_type != Variant::Type::FLOAT) {
                ERR_PRINT("Non-numerical value in the array.");
                goto clean_up_list;
            }

            switch(p_value_type) {
                case IREE_VM_VALUE_TYPE_I8: {
                    ERR_PRINT("Unable to convert value in array to int8.");
                    goto clean_up_list;
                }
                case IREE_VM_VALUE_TYPE_I16: {
                    ERR_PRINT("Unable to convert value in array to int16.");
                    goto clean_up_list;
                }
                case IREE_VM_VALUE_TYPE_I32: {
                    if(element_type == Variant::Type::FLOAT) {
#ifdef REAL_T_IS_DOUBLE
                        ERR_PRINT("Unable to convert double to int32.")
                        goto clean_up_list;
#else
                        WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                        variant = iree_vm_make_variant_value(
                            iree_vm_value_make_i32(float(element)) // WARNING: might lost precision.
                        );
#endif
                    } else variant = iree_vm_make_variant_value(
                        iree_vm_value_make_i32(int32_t(element))
                    );
                    break;
                }
                case IREE_VM_VALUE_TYPE_I64: {
                    if(element_type == Variant::Type::FLOAT) {
                        WARN_PRINT_ONCE("Conversion from float to int, might lost preciesion.");
                        variant = iree_vm_make_variant_value(
#ifdef REAL_T_IS_DOUBLE
                            iree_vm_value_make_i64(double(element)) // WARNING: might lost precision.
#else 
                            iree_vm_value_make_i64(float(element)) // WARNING: might lost precision.
#endif
                        );
                    } else variant = iree_vm_make_variant_value(
                        iree_vm_value_make_i64(int64_t(element))
                    );
                    break;
                }
                case IREE_VM_VALUE_TYPE_F32: {
                    variant = iree_vm_make_variant_value(
#ifdef REAL_T_IS_DOUBLE
                        ERR_PRINT("Unable to convert double to float.")
                        goto clean_up_list;
#else
                        iree_vm_value_make_f32(float(element))
#endif
                    );
                    break;
                }
                case IREE_VM_VALUE_TYPE_F64 : {
                    variant = iree_vm_make_variant_value(
#ifdef REAL_T_IS_DOUBLE
                        iree_vm_value_make_f64(double(element))
#else 
                        iree_vm_value_make_f64(float(element))
#endif
                    );
                    break;
                }
                case IREE_VM_VALUE_TYPE_NONE:
                case IREE_VM_VALUE_TYPE_COUNT:
                default:
                    ERR_PRINT("Unable to convert value to unknown type for IREE list.");
                    goto clean_up_list;
            }

            if(iree_vm_list_push_variant_move(
                list, &variant
            )) {
                ERR_PRINT("Unable to create IREE list.");
                goto clean_up_list;
            }

        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const PackedByteArray& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            p_value.size(), iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(const int8_t byte : p_value) {
        iree_vm_value_t value;

        switch(p_value_type) {
            case IREE_VM_VALUE_TYPE_I8:
                value = iree_vm_value_make_i8(byte);
                break;
            case IREE_VM_VALUE_TYPE_I16:
                value = iree_vm_value_make_i16(byte);
                break;
            case IREE_VM_VALUE_TYPE_I32:
                value = iree_vm_value_make_i32(byte);
                break;
            case IREE_VM_VALUE_TYPE_I64:
                value = iree_vm_value_make_i64(byte);
                break;
            case IREE_VM_VALUE_TYPE_F32: 
                value = iree_vm_value_make_f32(byte);
                break;
            case IREE_VM_VALUE_TYPE_F64 : 
                value = iree_vm_value_make_f64(byte);
                break;
            case IREE_VM_VALUE_TYPE_NONE:
            case IREE_VM_VALUE_TYPE_COUNT:
            default:
                ERR_PRINT("Unable to convert value to unknown type for IREE list.");
                goto clean_up_list;
        }

        if(iree_vm_list_push_value(list, &value)) {
            iree_vm_list_release(list);
            ERR_PRINT("Unable to insert data into IREE list.");
            return nullptr;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const PackedInt32Array& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            p_value.size(), iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(const int32_t number: p_value) {
        iree_vm_value_t value;

        switch(p_value_type) {
            case IREE_VM_VALUE_TYPE_I32:
                value = iree_vm_value_make_i32(number);
                break;
            case IREE_VM_VALUE_TYPE_I64:
                value = iree_vm_value_make_i64(number);
                break;
            case IREE_VM_VALUE_TYPE_F32: 
                value = iree_vm_value_make_f32(number);
                break;
            case IREE_VM_VALUE_TYPE_F64 : 
                value = iree_vm_value_make_f64(number);
                break;
            case IREE_VM_VALUE_TYPE_I8:
            case IREE_VM_VALUE_TYPE_I16:
                ERR_PRINT("Unable to convert int32 to smaller type.");
                goto clean_up_list;
                break;
            case IREE_VM_VALUE_TYPE_NONE:
            case IREE_VM_VALUE_TYPE_COUNT:
            default:
                ERR_PRINT("Unable to convert value to unknown type for IREE list.");
                goto clean_up_list;
        }

        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to insert data into IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const PackedInt64Array& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            p_value.size(), iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(const int64_t number: p_value) {
        iree_vm_value_t value;

        switch(p_value_type) {
            case IREE_VM_VALUE_TYPE_I64:
                value = iree_vm_value_make_i64(number);
                break;
            case IREE_VM_VALUE_TYPE_F64 : 
                value = iree_vm_value_make_f64(number);
                break;
            case IREE_VM_VALUE_TYPE_I8:
            case IREE_VM_VALUE_TYPE_I16:
            case IREE_VM_VALUE_TYPE_I32:
            case IREE_VM_VALUE_TYPE_F32:
                ERR_PRINT("Unable to convert float to smaller type.");
                goto clean_up_list;
                break;
            case IREE_VM_VALUE_TYPE_NONE:
            case IREE_VM_VALUE_TYPE_COUNT:
            default:
                ERR_PRINT("Unable to convert value to unknown type for IREE list.");
                goto clean_up_list;
        }

        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to insert data into IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const PackedFloat32Array& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            p_value.size(), iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(const float number: p_value) {
        iree_vm_value_t value;

        switch(p_value_type) {
            case IREE_VM_VALUE_TYPE_F32: 
                value = iree_vm_value_make_f32(number);
                break;
            case IREE_VM_VALUE_TYPE_F64 : 
                value = iree_vm_value_make_f64(number);
                break;
            case IREE_VM_VALUE_TYPE_I8:
            case IREE_VM_VALUE_TYPE_I16:
            case IREE_VM_VALUE_TYPE_I32:
            case IREE_VM_VALUE_TYPE_I64:
                ERR_PRINT("Unable to convert float to smaller type.");
                goto clean_up_list;
                break;
            case IREE_VM_VALUE_TYPE_NONE:
            case IREE_VM_VALUE_TYPE_COUNT:
            default:
                ERR_PRINT("Unable to convert value to unknown type for IREE list.");
                goto clean_up_list;
        }

        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to insert data into IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const PackedFloat64Array& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            p_value.size(), iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(const double number: p_value) {
        iree_vm_value_t value;

        switch(p_value_type) {
            case IREE_VM_VALUE_TYPE_F64 : 
                value = iree_vm_value_make_f64(number);
                break;
            case IREE_VM_VALUE_TYPE_I8:
            case IREE_VM_VALUE_TYPE_I16:
            case IREE_VM_VALUE_TYPE_I32:
            case IREE_VM_VALUE_TYPE_I64:
            case IREE_VM_VALUE_TYPE_F32:
                ERR_PRINT("Unable to convert int32 to smaller type.");
                goto clean_up_list;
                break;
            case IREE_VM_VALUE_TYPE_NONE:
            case IREE_VM_VALUE_TYPE_COUNT:
            default:
                ERR_PRINT("Unable to convert value to unknown type for IREE list.");
                goto clean_up_list;
        }

        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to insert data into IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const PackedVector2Array& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_ref_type_def(IREE_VM_REF_TYPE_ANY), 
            p_value.size(), iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(const Vector2& number: p_value) {
        iree_vm_list_t* nested_list = value_to_raw_list(number, p_value_type);
        if(nested_list == nullptr) goto clean_up_list;
        iree_vm_ref_t retained = iree_vm_list_retain_ref(nested_list);
        iree_vm_list_push_ref_move(list, &retained);
        iree_vm_list_release(nested_list);
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const PackedVector3Array& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_ref_type_def(IREE_VM_REF_TYPE_ANY), 
            p_value.size(), iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(const Vector3& number : p_value) {
        iree_vm_list_t* nested_list = value_to_raw_list(number, p_value_type);
        if(nested_list == nullptr) goto clean_up_list;
        iree_vm_ref_t retained = iree_vm_list_retain_ref(nested_list);
        iree_vm_list_push_ref_move(list, &retained);
        iree_vm_list_release(nested_list);
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const PackedColorArray& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_ref_type_def(IREE_VM_REF_TYPE_ANY), 
            p_value.size(), iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(const Color& number : p_value) {
        iree_vm_list_t* nested_list = value_to_raw_list(number, p_value_type);
        if(nested_list == nullptr) goto clean_up_list;
        iree_vm_ref_t retained = iree_vm_list_retain_ref(nested_list);
        iree_vm_list_push_ref_move(list, &retained);
        iree_vm_list_release(nested_list);
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const Vector2& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");
    ERR_FAIL_COND_V_MSG(
        p_value_type == IREE_VM_VALUE_TYPE_I8 ||
        p_value_type == IREE_VM_VALUE_TYPE_I16 ||
        p_value_type == IREE_VM_VALUE_TYPE_I32 ||
        p_value_type == IREE_VM_VALUE_TYPE_I64, nullptr, "Unable to convert float to smaller type."
    );
#ifdef REAL_T_IS_DOUBLE
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_F32, nullptr, "Unable to convert float to smaller type.");
#endif

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            2, iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(int i = 0; i < p_value.AXIS_COUNT; i++) {
        iree_vm_value_t value;
#ifdef REAL_T_IS_DOUBLE
        value = iree_vm_value_make_f64(p_value.coord[i]);
#else
        value = iree_vm_value_make_f32(p_value.coord[i]);
#endif
        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to create IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const Vector2i& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");
    ERR_FAIL_COND_V_MSG(
        p_value_type == IREE_VM_VALUE_TYPE_I8 ||
        p_value_type == IREE_VM_VALUE_TYPE_I16, nullptr, "Unable to convert float to smaller type."
    );

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            2, iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(int i = 0; i < p_value.AXIS_COUNT; i++){
        iree_vm_value_t value;

        switch(p_value_type) {
            case IREE_VM_VALUE_TYPE_I32:
                value = iree_vm_value_make_i32(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_I64:
                value = iree_vm_value_make_i64(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_F32: 
                value = iree_vm_value_make_f32(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_F64 : 
                value = iree_vm_value_make_f64(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_I8:
            case IREE_VM_VALUE_TYPE_I16:
                ERR_PRINT("Unable to convert int32 to smaller type.");
                goto clean_up_list;
                break;
            case IREE_VM_VALUE_TYPE_NONE:
            case IREE_VM_VALUE_TYPE_COUNT:
            default:
                ERR_PRINT("Unable to convert value to unknown type for IREE list.");
                goto clean_up_list;
        }
        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to create IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const Vector3& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");
    ERR_FAIL_COND_V_MSG(
        p_value_type == IREE_VM_VALUE_TYPE_I8 ||
        p_value_type == IREE_VM_VALUE_TYPE_I16 ||
        p_value_type == IREE_VM_VALUE_TYPE_I32 ||
        p_value_type == IREE_VM_VALUE_TYPE_I64, nullptr, "Unable to convert float to smaller type."
    );
#ifdef REAL_T_IS_DOUBLE
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_F32, nullptr, "Unable to convert float to smaller type.");
#endif

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            2, iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(int i = 0; i < p_value.AXIS_COUNT; i++) {
        iree_vm_value_t value;
#ifdef REAL_T_IS_DOUBLE
        value = iree_vm_value_make_f64(p_value.coord[i]);
#else
        value = iree_vm_value_make_f32(p_value.coord[i]);
#endif
        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to create IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const Vector3i& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");
    ERR_FAIL_COND_V_MSG(
        p_value_type == IREE_VM_VALUE_TYPE_I8 ||
        p_value_type == IREE_VM_VALUE_TYPE_I16, nullptr, "Unable to convert float to smaller type."
    );

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            2, iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(int i = 0; i < p_value.AXIS_COUNT; i++){
        iree_vm_value_t value;

        switch(p_value_type) {
            case IREE_VM_VALUE_TYPE_I32:
                value = iree_vm_value_make_i32(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_I64:
                value = iree_vm_value_make_i64(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_F32: 
                value = iree_vm_value_make_f32(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_F64 : 
                value = iree_vm_value_make_f64(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_I8:
            case IREE_VM_VALUE_TYPE_I16:
                ERR_PRINT("Unable to convert int32 to smaller type.");
                goto clean_up_list;
                break;
            case IREE_VM_VALUE_TYPE_NONE:
            case IREE_VM_VALUE_TYPE_COUNT:
            default:
                ERR_PRINT("Unable to convert value to unknown type for IREE list.");
                goto clean_up_list;
        }
        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to create IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const Vector4& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");
    ERR_FAIL_COND_V_MSG(
        p_value_type == IREE_VM_VALUE_TYPE_I8 ||
        p_value_type == IREE_VM_VALUE_TYPE_I16 ||
        p_value_type == IREE_VM_VALUE_TYPE_I32 ||
        p_value_type == IREE_VM_VALUE_TYPE_I64, nullptr, "Unable to convert float to smaller type."
    );
#ifdef REAL_T_IS_DOUBLE
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_F32, nullptr, "Unable to convert float to smaller type.");
#endif

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            2, iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(int i = 0; i < p_value.AXIS_COUNT; i++) {
        iree_vm_value_t value;
#ifdef REAL_T_IS_DOUBLE
        value = iree_vm_value_make_f64(p_value.components[i]);
#else
        value = iree_vm_value_make_f32(p_value.components[i]);
#endif
        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to create IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const Vector4i& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");
    ERR_FAIL_COND_V_MSG(
        p_value_type == IREE_VM_VALUE_TYPE_I8 ||
        p_value_type == IREE_VM_VALUE_TYPE_I16, nullptr, "Unable to convert float to smaller type."
    );

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            2, iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(int i = 0; i < p_value.AXIS_COUNT; i++){
        iree_vm_value_t value;

        switch(p_value_type) {
            case IREE_VM_VALUE_TYPE_I32:
                value = iree_vm_value_make_i32(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_I64:
                value = iree_vm_value_make_i64(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_F32: 
                value = iree_vm_value_make_f32(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_F64 : 
                value = iree_vm_value_make_f64(p_value.coord[i]);
                break;
            case IREE_VM_VALUE_TYPE_I8:
            case IREE_VM_VALUE_TYPE_I16:
                ERR_PRINT("Unable to convert int32 to smaller type.");
                goto clean_up_list;
                break;
            case IREE_VM_VALUE_TYPE_NONE:
            case IREE_VM_VALUE_TYPE_COUNT:
            default:
                ERR_PRINT("Unable to convert value to unknown type for IREE list.");
                goto clean_up_list;
        }
        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to create IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

iree_vm_list_t* IREEData::value_to_raw_list(const Color& p_value, iree_vm_value_type_t p_value_type) {
    ERR_FAIL_COND_V_MSG(p_value_type == IREE_VM_VALUE_TYPE_NONE, nullptr, "Unable to produce IREE list with undefined type.");
    ERR_FAIL_COND_V_MSG(
        p_value_type == IREE_VM_VALUE_TYPE_I8 ||
        p_value_type == IREE_VM_VALUE_TYPE_I16 ||
        p_value_type == IREE_VM_VALUE_TYPE_I32 ||
        p_value_type == IREE_VM_VALUE_TYPE_I64, nullptr, "Unable to convert float to smaller type."
    );

    iree_vm_list_t* list = nullptr;

    ERR_FAIL_COND_V_MSG(
        iree_vm_list_create(
            iree_vm_make_value_type_def(p_value_type), 
            2, iree_allocator_system(), &list
        ),
        nullptr, "Unable to create IREE list."
    );

    for(int i = 0; i < 4; i++){
        iree_vm_value_t value;

        switch(p_value_type) {
            case IREE_VM_VALUE_TYPE_F32: 
                value = iree_vm_value_make_f32(p_value.components[i]);
                break;
            case IREE_VM_VALUE_TYPE_F64 : 
                value = iree_vm_value_make_f64(p_value.components[i]);
                break;
            case IREE_VM_VALUE_TYPE_I8:
            case IREE_VM_VALUE_TYPE_I16:
            case IREE_VM_VALUE_TYPE_I32:
            case IREE_VM_VALUE_TYPE_I64:
                ERR_PRINT("Unable to convert float to smaller type.");
                goto clean_up_list;
                break;
            case IREE_VM_VALUE_TYPE_NONE:
            case IREE_VM_VALUE_TYPE_COUNT:
            default:
                ERR_PRINT("Unable to convert value to unknown type for IREE list.");
                goto clean_up_list;
        }
        if(iree_vm_list_push_value(list, &value)) {
            ERR_PRINT("Unable to create IREE list.");
            goto clean_up_list;
        }
    }

    return list;

clean_up_list:
    iree_vm_list_release(list);
    return nullptr;
}

Array IREEData::raw_list_to_array(const iree_vm_list_t* p_list) {
    Array result;

    iree_host_size_t list_size = iree_vm_list_size(p_list);

    for(iree_host_size_t i = 0; i < list_size; i++) {
        iree_vm_variant_t variant;
        ERR_FAIL_COND_V_MSG(
            iree_vm_list_get_variant_retain(p_list, i, &variant),
            Array(), "Unable to retrieve data from IREE list."
        );

        // A list.
        if(iree_vm_variant_is_ref(variant)) {
            if(variant.ref.type != iree_vm_list_type()) {
                iree_vm_ref_release(&variant.ref);
                ERR_PRINT("Expecting list but otherwise is retrieved.");
                return Array();
            }

            Array nested_array = raw_list_to_array((iree_vm_list_t*)variant.ref.ptr);
            
            if(nested_array.is_empty()) {
                iree_vm_ref_release(&variant.ref);
                return Array();
            }

            result.append(nested_array);

            iree_vm_ref_release(&variant.ref);
        } 
        // A number.
        else {
            iree_vm_value_t value = iree_vm_variant_value(variant);
            switch(value.type) {
                case IREE_VM_VALUE_TYPE_I8:
                    result.append(value.i8);
                    break;
                case IREE_VM_VALUE_TYPE_I16:
                    result.append(value.i16);
                    break;
                case IREE_VM_VALUE_TYPE_I32:
                    result.append(value.i32);
                    break;
                case IREE_VM_VALUE_TYPE_I64:
                    result.append(value.i64);
                    break;
                case IREE_VM_VALUE_TYPE_F32:
                    result.append(value.f32);
                    break;
                case IREE_VM_VALUE_TYPE_F64:
                    result.append(value.f64);
                    break;
                case IREE_VM_VALUE_TYPE_NONE:
                case IREE_VM_VALUE_TYPE_COUNT:
                default:
                    ERR_PRINT("Unable to retrieve element from IREE list.");
                    return Array();
            }
        }
    }
    
    return result;
}

IREEData::operator Array() {
    return to_array();
}

Array IREEData::to_array() const {
    if(data == nullptr) return Array();
    return raw_list_to_array(data);
}

IREEData::IREEData()
:
    data(nullptr)
{}

IREEData::~IREEData() {
    if(data != nullptr) {iree_vm_list_release(data); data = nullptr;}
}