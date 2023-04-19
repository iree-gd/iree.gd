#ifndef IREE_DATA_H
#define IREE_DATA_H

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

using namespace godot;

/* Interface between Godot and IREE bytecode in term of data. */
class IREEData : public RefCounted {
    GDCLASS(IREEData, RefCounted)

    friend class IREEModule;
private:
    iree_vm_list_t* data;

    IREEData(iree_vm_list_t* p_data);
protected:
    static void _bind_methods();

public:
    // Convert Godot variant to IREE list.
    // Returned list is handled by the caller.
    // Return `nullptr` on fail.
    static iree_vm_list_t* value_to_raw_list(const Variant& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const Array& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const PackedByteArray& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const PackedInt32Array& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const PackedInt64Array& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const PackedFloat32Array& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const PackedFloat64Array& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const PackedVector2Array& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const PackedVector3Array& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const PackedColorArray& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const Vector2& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const Vector2i& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const Vector3& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const Vector3i& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const Vector4& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const Vector4i& p_value, iree_vm_value_type_t p_value_type);
    static iree_vm_list_t* value_to_raw_list(const Color& p_value, iree_vm_value_type_t p_value_type);

    // Convert IREE list to Godot array.
    static Array raw_list_to_array(const iree_vm_list_t* p_list);

    operator Array();


    IREEData();
    ~IREEData();

    Array to_array() const;
};

#endif //IREE_DATA_H