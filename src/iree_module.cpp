#include "iree_module.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/file_access.hpp>

#include <iree/modules/hal/types.h>
#include <iree/modules/hal/module.h>
#include <iree/vm/bytecode/module.h>

#include "iree_instance.h"

using namespace godot;

void IREEModule::unload() {
	if(context != nullptr) {iree_vm_context_release(context); context = nullptr;}
	if(bytecode != nullptr) {iree_vm_module_release(bytecode);  bytecode = nullptr;}
    load_path = "";
    entry.fn = {0};
    entry.symbol = "";
}

bool IREEModule::is_loaded() {
    return bytecode && context;
}

bool IREEModule::is_init() {
    return is_loaded() && entry.symbol.length() != 0;
}

Error IREEModule::load(const String& p_path) {
    // Get hal module.
    iree_vm_module_t* const hal_module = IREEInstance::get_hal_module();
    ERR_FAIL_NULL_V(hal_module, ERR_CANT_CREATE);

    // Get instance.
    iree_vm_instance_t* const instance = IREEInstance::get_vm_instance();
    ERR_FAIL_NULL_V(instance, ERR_CANT_CREATE);

    // Unload old data.
    unload();

    // Read file content.
    PackedByteArray bytecode_data = FileAccess::get_file_as_bytes(p_path);

    // Create a module.
    iree_vm_module_t* new_bytecode = nullptr;
    ERR_FAIL_COND_V_MSG(iree_vm_bytecode_module_create(
        instance, iree_const_byte_span_t{
            bytecode_data.ptr(), 
            (iree_host_size_t)bytecode_data.size()
        },
        iree_allocator_null(), iree_allocator_system(), &new_bytecode
    ), ERR_CANT_CREATE, "Unable to load IREE module.");
    bytecode = new_bytecode;

    // Create a context.
    iree_vm_context_t* new_context = nullptr;
    iree_vm_module_t* modules[2] = {hal_module, bytecode};
    ERR_FAIL_COND_V_MSG(iree_vm_context_create_with_modules(
        instance, IREE_VM_CONTEXT_FLAG_NONE,
        IREE_ARRAYSIZE(modules), modules,
        iree_allocator_system(), &new_context
    ) , ERR_CANT_CREATE, "Unable to create IREE context.");
    context = new_context;

    load_path = p_path;

    notify_property_list_changed();
    emit_changed();
    return OK;
}

String IREEModule::get_load_path() const {
    return load_path;
}

Error IREEModule::set_entry_point(const String& p_entry) {
    ERR_FAIL_COND_V_MSG(!is_loaded(), ERR_CANT_OPEN, "IREE bytecode is not loaded.");

    iree_vm_function_t fn = {0};
    ERR_FAIL_COND_V_MSG(iree_vm_context_resolve_function(
        context, iree_string_view_t{
            .data = (const char*)p_entry.ptr(), 
            .size = (unsigned long)p_entry.length()
        }, 
        &fn
    ), ERR_CANT_RESOLVE, vformat("Unable to find function '%s' in bytecode.", p_entry));

    entry.fn = fn;
    entry.symbol = p_entry;

    notify_property_list_changed();
    emit_changed();
    return OK;
}

String IREEModule::get_entry_point() const {
    return entry.symbol;
}

PackedFloat32Array IREEModule::call(const PackedFloat32Array& p_inputs) const {
    iree_vm_list_t* inputs = nullptr;
    iree_vm_type_def_t value_type = iree_vm_make_value_type_def(IREE_VM_VALUE_TYPE_F32);
    iree_vm_list_t* outputs = nullptr;
    iree_host_size_t outputs_size = 0;
    PackedFloat32Array result;

    /// Convert input array to iree list.
    
    // Create a new iree list for inputs.
    ERR_FAIL_COND_V_MSG(iree_vm_list_create(
        value_type, p_inputs.size(), 
        iree_allocator_system(), &inputs
    ), result, "Unable to allocate memory for IREE input list.");

    // Insert values to list.
    for(int i = 0; i < p_inputs.size(); i++) {
        const iree_vm_value_t value = {
            .type = IREE_VM_VALUE_TYPE_F64,
            .f32 = p_inputs[i]
        };
        if(iree_vm_list_push_value(inputs, &value)) {
            ERR_PRINT("Unable to convert Godot array to IREE list.");
            goto clean_up_inputs;
        }
    }

    /// Prepare for output.

    // Create a new iree list for outputs.
    if(iree_vm_list_create(
        value_type, p_inputs.size(), 
        iree_allocator_system(), &inputs
    )) {
        ERR_PRINT("Unable to allocate memory for IREE output list.");
        goto clean_up_inputs;
    }

    /// Invoke.
    if(iree_vm_invoke(
        context, entry.fn, IREE_VM_INVOCATION_FLAG_NONE,
        /*policy=*/ NULL, inputs, outputs, iree_allocator_system()
    )) {
        ERR_PRINT(vformat("Unable to call IREE function '%s'.", entry.symbol));
        goto clean_up_outputs;
    }

    /// Convert output list to array.
    outputs_size = iree_vm_list_size(outputs);
    result.resize(outputs_size);
    for (int i = 0; i < outputs_size; i++) {
        iree_vm_value_t value;
        if(iree_vm_list_get_value(outputs, i, &value)) {
            ERR_PRINT("Unable to convert IREE list to Godot array.");
            result.clear();
            goto clean_up_outputs;
        }
        result[i] = value.f32;
    }

clean_up_outputs:
    iree_vm_list_release(outputs);

clean_up_inputs:
    iree_vm_list_release(inputs);

    return result;
}

void IREEModule::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_init"), &IREEModule::is_init);

    ClassDB::bind_method(D_METHOD("load", "path"), &IREEModule::load);
    ClassDB::bind_method(D_METHOD("get_load_path"), &IREEModule::get_load_path);

    ClassDB::bind_method(D_METHOD("set_entry_point", "symbol"), &IREEModule::set_entry_point);
    ClassDB::bind_method(D_METHOD("get_entry_point"), &IREEModule::get_entry_point);

    ClassDB::bind_method(D_METHOD("call", "inputs"), &IREEModule::call);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "entry_point", PROPERTY_HINT_NONE, "Entry function name"), "set_entry_point", "get_entry_point");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "load_path", PROPERTY_HINT_FILE, "*.vmfb"), "load", "get_load_path");
}

IREEModule::IREEModule()
:
    bytecode(nullptr),
    context(nullptr),
    load_path(""),
    entry()
{ }

IREEModule::~IREEModule() { unload(); }