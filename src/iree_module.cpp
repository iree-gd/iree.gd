#include "iree_module.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/file_access.hpp>

#include <iree/modules/hal/types.h>
#include <iree/modules/hal/module.h>
#include <iree/vm/bytecode/module.h>

#include "iree.h"

using namespace godot;

void IREEModule::unload() {
	if(context != nullptr) {iree_vm_context_release(context); context = nullptr;}
	if(bytecode != nullptr) {iree_vm_module_release(bytecode);  bytecode = nullptr;}
    load_path = "";
}

bool IREEModule::is_loaded() {
    return bytecode && context;
}

Error IREEModule::load(const String& p_path) {
    // Get hal module.
    iree_vm_module_t* const hal_module = IREE::get_hal_module();
    ERR_FAIL_NULL_V(hal_module, ERR_CANT_CREATE);

    // Get instance.
    iree_vm_instance_t* const instance = IREE::get_vm_instance();
    ERR_FAIL_NULL_V(instance, ERR_CANT_CREATE);

    // Unload old data.
    unload();

    // Read file content.
    bytecode_data = FileAccess::get_file_as_bytes(p_path);

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

IREEData IREEModule::call_vmfb(const Variant** p_argv, GDExtensionInt p_argc, GDExtensionCallError& r_error) const {
    if(p_argc < 1) {
        r_error.error = GDExtensionCallErrorType::GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS;
        r_error.argument = 0;
        return IREEData();
    }

    if(p_argv[0]->get_type() != Variant::Type::STRING || p_argv[0]->get_type() != Variant::Type::STRING_NAME) {
        r_error.error = GDExtensionCallErrorType::GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
        r_error.argument = 0;
        r_error.expected = Variant::Type::STRING_NAME;
        return IREEData();
    }

    const String func_name = *p_argv[0];
    iree_vm_function_t func = {0};

    // Query function.
    if(iree_vm_context_resolve_function(
        context, iree_string_view_t{
            .data = (const char*)func_name.ptr(), 
            .size = (unsigned long)func_name.length()
        }, 
        &func
    )) {
        r_error.error = GDExtensionCallErrorType::GDEXTENSION_CALL_ERROR_INVALID_METHOD;
        r_error.argument = 0;
        return IREEData();
    }
     
    // Convert inputs.
    Array func_args;
    for(GDExtensionInt i = 1; i < p_argc; i++)
        func_args.append(*p_argv[i]);
    iree_vm_list_t* inputs = IREEData::value_to_raw_list(func_args, IREE_VM_VALUE_TYPE_F32); // TODO: Figure out the type of input.
    
    if(inputs == nullptr) {
        r_error.error = GDExtensionCallErrorType::GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
        r_error.argument = 1;
        return IREEData();
    }

    // Create a new iree list for outputs.
    IREEData results;
    iree_vm_list_t* outputs = nullptr;
    if(iree_vm_list_create(
        iree_vm_make_ref_type_def(IREE_VM_REF_TYPE_NULL), func_args.size(), 
        iree_allocator_system(), &inputs
    )) {
        ERR_PRINT("Unable to allocate memory for IREE output list.");
        goto clean_up_inputs;
    }

    // Call.
    if(iree_vm_invoke(
        context, func, IREE_VM_INVOCATION_FLAG_NONE,
        /*policy=*/ NULL, inputs, outputs, iree_allocator_system()
    )) {
        ERR_PRINT(vformat("Unable to call IREE function '%s'.", func_name));
        goto clean_up_outputs;
    }

    // Setup result.
    results.data = outputs;

clean_up_outputs:
    iree_vm_list_release(outputs);

clean_up_inputs:
    iree_vm_list_release(inputs);
    return results;

}

void IREEModule::_bind_methods() {
    {
        MethodInfo mi;
        mi.name = "call_vmfb";
        mi.arguments.push_back(PropertyInfo(Variant::Type::STRING, "func_name"));
        ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "call_vmfb", &IREEModule::call_vmfb, mi);
    }

    ClassDB::bind_method(D_METHOD("load", "path"), &IREEModule::load);
    ClassDB::bind_method(D_METHOD("get_load_path"), &IREEModule::get_load_path);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "load_path", PROPERTY_HINT_FILE, "*.vmfb"), "load", "get_load_path");
}

IREEModule::IREEModule()
:
    bytecode_data(),
    bytecode(nullptr),
    context(nullptr),
    load_path("")
{ }

IREEModule::~IREEModule() { unload(); }