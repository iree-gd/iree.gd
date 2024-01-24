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
#include "iree_error.h"

using namespace godot;

Error IREEModule::capture()
{
    ERR_FAIL_COND_V_MSG(bytecode_data.size() == 0, ERR_INVALID_DATA, "Empty bytecode is forbidden.");

    // Get instance.
    iree_vm_instance_t *const instance = IREEInstance::borrow_singleton()->borrow_assured_vm_instance();
    ERR_FAIL_NULL_V(instance, ERR_CANT_CREATE);

    // Get hal module.
    iree_vm_module_t *const hal_module = IREEInstance::borrow_singleton()->borrow_assured_hal_module();
    ERR_FAIL_NULL_V(hal_module, ERR_CANT_CREATE);

    release();

    // Create a module.
    iree_const_byte_span_t byte_span = {0};
    byte_span.data = bytecode_data.ptr();
    byte_span.data_length = (iree_host_size_t)bytecode_data.size();
    iree_vm_module_t *new_bytecode_module = nullptr;
    IREE_ERR_V_MSG(
        iree_vm_bytecode_module_create(
            instance, byte_span,
            iree_allocator_null(),
            iree_allocator_system(),
            &new_bytecode_module),
        ERR_CANT_CREATE, "Unable to load IREE module.");
    bytecode_module = new_bytecode_module;

    // Create a context.
    iree_vm_context_t *new_context = nullptr;
    iree_vm_module_t *modules[2] = {hal_module, bytecode_module};

    IREE_ERR_V_MSG(
        iree_vm_context_create_with_modules(
            instance, IREE_VM_CONTEXT_FLAG_NONE,
            IREE_ARRAYSIZE(modules), modules,
            iree_allocator_system(), &new_context),
        ERR_CANT_CREATE, "Unable to create IREE context.");
    context = new_context;

    return OK;
}

void IREEModule::release()
{
    if (context != nullptr)
    {
        iree_vm_context_release(context);
        context = nullptr;
    }
    if (bytecode_module != nullptr)
    {
        iree_vm_module_release(bytecode_module);
        bytecode_module = nullptr;
    }
}

Array IREEModule::process()
{
    if (!is_captured())
    {
        Error status = capture();
        if (status != OK)
            return Array();
    }

    ERR_FAIL_COND_V_MSG(!(bytecode_module && context), Array(), "IREE Module is not loaded.");

    PackedByteArray raw_func_name = func_name.to_utf8_buffer();
    iree_vm_function_t func = {0};

    // Query function.
    IREE_ERR_V_MSG(
        iree_vm_context_resolve_function(
            context, iree_string_view_t{.data = (const char *)raw_func_name.ptr(), .size = (unsigned long)raw_func_name.size()},
            &func),
        Array(),
        vformat("Unable to find function '%s' in module bytecode.", func_name));

    // Convert inputs.
    IREEList inputs;
    inputs.capture(10);
    ERR_FAIL_COND_V(!inputs.is_captured(), Array());

    for (int64_t i = 0; i < args.size(); i++)
    {
        Variant arg = args[i];
        if (arg.get_type() != Variant::Type::OBJECT)
        {
            ERR_PRINT("Expecting only IREE Tensors as arguments.");
            return Array();
        }
        Object *obj = (Object *)arg;
        if (!obj->is_class("IREETensor"))
        {
            ERR_PRINT("Expecting only IREE Tensors as arguments.");
            return Array();
        }
        Ref<IREETensor> tensor = (Ref<IREETensor>)obj;
        ERR_FAIL_COND_V_MSG(tensor.is_null(), Array(), "Given IREE tensor is null.");
        ERR_FAIL_COND_V_MSG(!tensor->is_captured(), Array(), "Given IREE tensor is null.");
        inputs.append(*tensor.ptr());
    }

    // Create a new iree list for outputs.
    IREEList outputs;
    outputs.capture(10);
    ERR_FAIL_COND_V(!outputs.is_captured(), Array());

    // Call.
    IREE_ERR_V_MSG(
        iree_vm_invoke(
            context, func, IREE_VM_INVOCATION_FLAG_NONE,
            /*policy=*/NULL, inputs.borrow_vm_list(), outputs.borrow_vm_list(), iree_allocator_system()),
        Array(),
        vformat("Unable to call IREE function '%s'.", func_name));

    return outputs.get_tensors();
}

void IREEModule::process_via_signal()
{
    Array output = process();
    call_deferred("emit_signal", "completed", output);
}

void IREEModule::on_process_completed(const Array &p_result)
{
    thread->wait_to_finish();
}

bool IREEModule::is_captured() const
{
    return bytecode_module && context;
}

void IREEModule::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("load", "path"), &IREEModule::load);
    ClassDB::bind_method(D_METHOD("unload"), &IREEModule::unload);
    ClassDB::bind_method(D_METHOD("bind", "func_name", "args"), &IREEModule::bind);
    ClassDB::bind_method(D_METHOD("call_module"), &IREEModule::call_module);
    ClassDB::bind_method(D_METHOD("call_module_async"), &IREEModule::call_module_async);

    ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::ARRAY, "result", PROPERTY_HINT_ARRAY_TYPE)));
}

IREEModule::IREEModule()
    : bytecode_data(),
      bytecode_module(nullptr),
      context(nullptr),
      func_name(),
      args(),
      thread()
{
    thread.instantiate();
    call_deferred("connect", "completed", callable_mp(this, &IREEModule::on_process_completed));
}

IREEModule::IREEModule(IREEModule &&p_module)
    : bytecode_data(p_module.bytecode_data),
      bytecode_module(p_module.bytecode_module),
      context(p_module.context),
      func_name(p_module.func_name),
      args(p_module.args),
      thread(p_module.thread)
{
    p_module.bytecode_data.clear();
    p_module.bytecode_module = nullptr;
    p_module.context = nullptr;
    p_module.func_name = "";
    p_module.args.clear();
    p_module.thread.unref();
    p_module.call_deferred("disconnect", "completed", callable_mp(this, &IREEModule::on_process_completed));

    call_deferred("connect", "completed", callable_mp(this, &IREEModule::on_process_completed));
}

IREEModule::~IREEModule()
{
    if (thread->is_alive())
        thread->wait_to_finish();
    unload();
}

Error IREEModule::load(const String &p_path)
{
    // Unload old data.
    unload();

    // Read file content.
    PackedByteArray new_bytecode_data;
    new_bytecode_data = FileAccess::get_file_as_bytes(p_path);
    ERR_FAIL_COND_V_MSG(new_bytecode_data.size() == 0, ERR_INVALID_DATA, "Empty bytecode is forbidden.");
    bytecode_data = new_bytecode_data;

    notify_property_list_changed();
    emit_changed();
    return OK;
}

void IREEModule::unload()
{
    release();
    bytecode_data.clear();
}

Ref<IREEModule> IREEModule::bind(const String &p_func_name, const Array &p_args)
{
    ERR_FAIL_COND_V_MSG(thread->is_alive(), this, "IREE Module is running asynchronously in the background.");
    func_name = p_func_name;
    args = p_args;
    return this;
}

Ref<IREEModule> IREEModule::call_module_async()
{
    ERR_FAIL_COND_V_MSG(thread->is_alive(), this, "IREE Module is running asynchronously in the background.");
    thread->start(callable_mp(this, &IREEModule::process_via_signal));
    return this;
}

Array IREEModule::call_module()
{
    ERR_FAIL_COND_V_MSG(thread->is_alive(), Array(), "IREE Module is running asynchronously in the background.");
    return process();
}
