#include "iree_instance.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/global_constants.hpp>

#include <iree/modules/hal/types.h>

using namespace godot;

IREEInstance* IREEInstance::singleton = nullptr;

iree_vm_instance_t *IREEInstance::get_vm_instance() {
    if(IREEInstance::singleton == nullptr)
        IREEInstance::singleton = new IREEInstance();

    return IREEInstance::singleton->instance;
}

IREEInstance::IREEInstance() 
:
    instance(nullptr)
{
	ERR_FAIL_COND_MSG(
		iree_vm_instance_create(IREE_VM_TYPE_CAPACITY_DEFAULT, iree_allocator_system(), &instance),
		"Unable create a VM instance."
	);
	ERR_FAIL_COND_MSG(
		iree_hal_module_register_all_types(instance),
		"Unable register HAL modules."
	);
}

IREEInstance::~IREEInstance() {
	if(instance != nullptr) {iree_vm_instance_release(instance); instance = nullptr;}
}
