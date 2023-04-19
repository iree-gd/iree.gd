#ifndef IREE_MODULE_H
#define IREE_MODULE_H

// We don't need windows.h in this example plugin but many others do, and it can
// lead to annoying situations due to the ton of macros it defines.
// So we include it and make sure CI warns us if we use something that conflicts
// with a Windows define.
#ifdef WIN32
#include <windows.h>
#endif

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>

#include <iree/base/api.h>
#include <iree/hal/api.h>
#include <iree/vm/api.h>

#include "iree_data.h"

using namespace godot;

class IREEModule : public Resource {
    GDCLASS(IREEModule, Resource)

private:
    PackedByteArray bytecode_data;
    iree_vm_module_t* bytecode;
	iree_vm_context_t* context;
    String load_path;

    void unload(); // unload the bytecode.

    // A vararg method for calling function in bytecode.
    // call_vmfb(func_name: String, ...)
    IREEData call_vmfb(const Variant** p_argv, GDExtensionInt p_argc, GDExtensionCallError& r_error) const;

protected:
    static void _bind_methods();

public:
    bool is_loaded(); // check whether the bytecode is loaded.

    Error load(const String& p_path);
    String get_load_path() const;

    IREEModule();
    ~IREEModule();
};

#endif //IREE_MODULE_H