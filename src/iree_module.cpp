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

Array IREEModule::_bind_call_vmfb(const Variant** p_argv, GDExtensionInt p_argc, GDExtensionCallError &m_error) {
    if(p_argc < 1) {
        m_error.argument = 0;
        m_error.error = GDExtensionCallErrorType::GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS;
        return Array();
    }

    if(p_argv[0]->get_type() != Variant::Type::STRING && p_argv[0]->get_type() != Variant::Type::STRING_NAME) {
        m_error.argument = 0;
        m_error.error = GDExtensionCallErrorType::GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
        m_error.expected = Variant::Type::STRING;
        return Array();
    }

    String func_name = String(*p_argv[0]);
    Array args;
    for(int i = 1; i < p_argc; i++) {
        const Variant value = *p_argv[i];

        if(value.get_type() != Variant::Type::OBJECT) {
            m_error.argument = i;
            m_error.error = GDExtensionCallErrorType::GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
            m_error.expected = Variant::Type::OBJECT;
            return Array();
        }

        const Object* object = value;

        if(object->get_class() != "IREEBufferView") {
            ERR_PRINT("Expecting IREEBufferView.");
            return Array();
        }

        args.append(value);
    }

    return call_vmfb(func_name, args).to_array();
}

bool IREEModule::is_loaded() const {
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

IREEIOList IREEModule::call_vmfb(const String& p_func_name, const Array& p_args) const {
    ERR_FAIL_COND_V_MSG(!is_loaded(), IREEIOList(), "IREEModule is not loaded.");

    iree_vm_function_t func = {0};

    PackedByteArray func_name = p_func_name.to_utf8_buffer();

    // Query function.
    ERR_FAIL_COND_V_MSG(iree_vm_context_resolve_function(
        context, iree_string_view_t{
            .data = (const char*)func_name.ptr(), 
            .size = (unsigned long)func_name.size()
        }, 
        &func
    ), IREEIOList(), vformat("Unable to find function '%s' in VMFB bytecode.", p_func_name));

    // Convert inputs.
    IREEIOList inputs; 
    inputs.init();
    ERR_FAIL_COND_V(!inputs.is_init(), IREEIOList());

    for(int64_t i = 0; i < p_args.size(); i++) {
        Ref<IREEBufferView> arg = (p_args[i]);
        ERR_FAIL_COND_V_MSG(arg.is_null(), IREEIOList(), "Given IREEBufferView is null.");
        inputs.append(arg);
    }

    // Create a new iree list for outputs.
    IREEIOList outputs; 
    outputs.init();
    ERR_FAIL_COND_V(!outputs.is_init(), IREEIOList());

    // Call.
    ERR_FAIL_COND_V_MSG(iree_vm_invoke(
        context, func, IREE_VM_INVOCATION_FLAG_NONE,
        /*policy=*/ NULL, inputs.ptr(), outputs.ptrw(), iree_allocator_system()
    ), IREEIOList(), vformat("Unable to call IREE function '%s'.", p_func_name));

    return outputs;
}

void IREEModule::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load", "path"), &IREEModule::load);
    ClassDB::bind_method(D_METHOD("get_load_path"), &IREEModule::get_load_path);

    {
        MethodInfo mi;
        mi.arguments.push_back(PropertyInfo(Variant::Type::STRING, "func_name"));
        mi.name = "call_vmfb";
        ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "call_vmfb", &IREEModule::_bind_call_vmfb, mi, {}, true);
    }

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