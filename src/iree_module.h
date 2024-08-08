#ifndef IREE_MODULE_H
#define IREE_MODULE_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <iree/base/api.h>
#include <iree/hal/api.h>
#include <iree/vm/api.h>

#include "iree_list.h"
#include "iree_tensor.h"

namespace godot {

class IREEModule : public Resource {
	GDCLASS(IREEModule, Resource)

private:
	PackedByteArray bytecode_data;
	iree_vm_module_t *bytecode_module;
	iree_vm_context_t *context;

	Ref<Thread> thread;

	Error capture();
	void release();
	Array process(const String &p_func_name, const Array &p_args);
	void process_return_via_signal(const String &p_func_name, const Array &p_args);
	void on_process_completed(const Array &p_result);

	bool is_captured() const;

protected:
	static void _bind_methods();

public:
	IREEModule();
	IREEModule(IREEModule &&p_module);
	~IREEModule();

	Error load(const String &p_path);
	void unload();
	Ref<IREEModule> call_module(const String &p_func_name, const Array &p_args);
};

} // namespace godot

#endif // IREE_MODULE_H
