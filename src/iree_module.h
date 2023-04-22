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

#include "iree_buffer_view.h"
#include "iree_io_list.h"

using namespace godot;

class IREEModule : public Resource {
    GDCLASS(IREEModule, Resource)

private:
    PackedByteArray bytecode_data;
    iree_vm_module_t* bytecode;
	iree_vm_context_t* context;
    String load_path;

    void unload(); // unload the bytecode.

protected:
    static void _bind_methods();

public:
    bool is_loaded() const; // check whether the bytecode is loaded.

    Error load(const String& p_path);
    String get_load_path() const;

    IREEIOList call_vmfb(const String& p_func_name, const Array& p_args) const;
    Array call_vmfb_array(const String& p_func_name, const Array& p_args) const;

    IREEModule();
    ~IREEModule();
};

#endif //IREE_MODULE_H