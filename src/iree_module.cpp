#include "iree_module.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <iree/modules/hal/types.h>
#include <iree/modules/hal/module.h>
#include <iree/vm/bytecode/module.h>

#include "iree_instance.h"

using namespace godot;

void IREEModule::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load", "path"), &IREEModule::load);
    ClassDB::bind_method(D_METHOD("unload"), &IREEModule::unload);
    ClassDB::bind_method(D_METHOD("get_load_path"), &IREEModule::get_load_path);
    ClassDB::bind_method(D_METHOD("call_module", "func_name", "args"), &IREEModule::call_module);
    
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "load_path", PROPERTY_HINT_FILE, "*.vmfb"), "load", "get_load_path");
}

IREEModule::IREEModule()
:
    bytecode_data(),
    bytecode(nullptr),
    context(nullptr),
    load_path("")
{}

IREEModule::IREEModule(IREEModule& p_module)
:
    bytecode_data(p_module.bytecode_data),
    bytecode(p_module.bytecode),
    context(p_module.context),
    load_path(p_module.load_path)
{
    iree_vm_module_retain(p_module.bytecode);
    iree_vm_context_retain(p_module.context);
}

IREEModule::IREEModule(IREEModule&& p_module)
:
    bytecode_data(p_module.bytecode_data),
    bytecode(p_module.bytecode),
    context(p_module.context),
    load_path(p_module.load_path)
{
    p_module.bytecode_data.clear();
    p_module.bytecode = nullptr;
    p_module.context = nullptr;
    p_module.load_path = "";
}

IREEModule::~IREEModule() { unload(); }

Error IREEModule::load(const String& p_path) {
    // Initialize status.
    iree_status_t status = iree_ok_status();

    // Get instance.
    iree_vm_instance_t* const instance = IREEInstance::borrow_singleton()->borrow_assured_vm_instance();
    ERR_FAIL_NULL_V_MSG(instance, ERR_CANT_CREATE, "Failure in `IREEModule::load`");

    // Get hal module.
    iree_vm_module_t* const hal_module = IREEInstance::borrow_singleton()->borrow_assured_hal_module();
    ERR_FAIL_NULL_V_MSG(hal_module, ERR_CANT_CREATE, "Failure in `IREEModule::load`");

    // Unload old data.
    unload();

    // Read file content.
    bytecode_data = FileAccess::get_file_as_bytes(p_path);

    // Create a module.
    iree_vm_module_t* new_bytecode = nullptr;
    if((status = iree_vm_bytecode_module_create(
        instance, iree_const_byte_span_t{
            bytecode_data.ptr(), 
            (iree_host_size_t)bytecode_data.size()
        },
        iree_allocator_null(), iree_allocator_system(), &new_bytecode))) 
    {
        Array format_array;
        format_array.append(String(iree_status_code_string(iree_status_code(status))));
        ERR_PRINT(String("Unable to load IREE module, IREE code: '{0}'.").format(format_array));
        iree_status_fprint(stderr, status);
        iree_status_free(status);
        return ERR_CANT_CREATE;
    }
    bytecode = new_bytecode;

    // Create a context.
    iree_vm_context_t* new_context = nullptr;
    iree_vm_module_t* modules[2] = {hal_module, bytecode};
    if((status = iree_vm_context_create_with_modules(
        instance, IREE_VM_CONTEXT_FLAG_NONE,
        IREE_ARRAYSIZE(modules), modules,
        iree_allocator_system(), &new_context))) 
    {
        Array format_array;
        format_array.append(String(iree_status_code_string(iree_status_code(status))));
        ERR_PRINT(String("Unable to create IREE contxt, IREE code: '{0}'.").format(format_array));
        iree_status_fprint(stderr, status);
        iree_status_free(status);
        return ERR_CANT_CREATE;
    }
    context = new_context;

    load_path = p_path;

    notify_property_list_changed();
    emit_changed();
    return OK;
}

void IREEModule::unload() {
	if(context != nullptr) {iree_vm_context_release(context); context = nullptr;}
	if(bytecode != nullptr) {iree_vm_module_release(bytecode);  bytecode = nullptr;}
    bytecode_data.clear(); //I don't know why but this causes crash.
    load_path = "";
}

bool IREEModule::is_loaded() const {
    return bytecode && context;
}

String IREEModule::get_load_path() const {
    return load_path;
}

Array IREEModule::call_module(const String& p_func_name, const Array& p_args) const {
    ERR_FAIL_COND_V_MSG(!is_loaded(), Array(), "IREE Module is not loaded.");

    PackedByteArray func_name = p_func_name.to_utf8_buffer();
    iree_vm_function_t func = {0};

    iree_status_t status = nullptr;

    // Query function.
    status = iree_vm_context_resolve_function(
        context, iree_string_view_t{
            .data = (const char*)func_name.ptr(), 
            .size = (unsigned long)func_name.size()
        }, 
        &func
    );
    ERR_FAIL_COND_V_MSG(
        status, 
        Array(), 
        vformat("Unable to find function '%s' in module bytecode, error code: %s.", p_func_name, iree_status_code_string(iree_status_code(status)))
    );


    // Convert inputs.
    IREEList inputs; 
    inputs.capture(10);
    ERR_FAIL_COND_V(inputs.is_null(), Array());

    for(int64_t i = 0; i < p_args.size(); i++) {
        Variant arg = p_args[i];
        if(arg.get_type() != Variant::Type::OBJECT) {
            ERR_PRINT("Expecting only IREE Tensors as arguments.");
            return Array();
        }
        Object* obj = (Object*)arg;
        if(!obj->is_class("IREETensor")) {
            ERR_PRINT("Expecting only IREE Tensors as arguments.");
            return Array();
        }
        Ref<IREETensor> tensor = (Ref<IREETensor>)obj;
        ERR_FAIL_COND_V_MSG(tensor.is_null(), Array(), "Given IREE tensor is null.");
        ERR_FAIL_COND_V_MSG(tensor->is_null(), Array(), "Given IREE tensor is null.");
        inputs.append(*tensor.ptr());
    }

    // Create a new iree list for outputs.
    IREEList outputs; 
    outputs.capture(10);
    ERR_FAIL_COND_V(outputs.is_null(), Array());

    // Call.
    status = iree_vm_invoke(
        context, func, IREE_VM_INVOCATION_FLAG_NONE,
        /*policy=*/ NULL, inputs.borrow_vm_list(), outputs.borrow_vm_list(), iree_allocator_system()
    );
    ERR_FAIL_COND_V_MSG(status, Array(), vformat("Unable to call IREE function '%s', error code: %s.", p_func_name, iree_status_code_string(iree_status_code(status))));

    return outputs.get_tensors();
}
