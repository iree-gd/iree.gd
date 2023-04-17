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

iree_hal_device_t* IREEInstance::get_device() {
    if(IREEInstance::singleton == nullptr)
        IREEInstance::singleton = new IREEInstance();

    return IREEInstance::singleton->device.device;
}

iree_vm_module_t* IREEInstance::get_hal_module() {
    if(IREEInstance::singleton == nullptr)
        IREEInstance::singleton = new IREEInstance();

    return IREEInstance::singleton->device.hal_module;
}

IREEInstance::IREEInstance() 
:
    instance(nullptr),
	device()
{
	ERR_FAIL_COND_MSG(
		iree_vm_instance_create(IREE_VM_TYPE_CAPACITY_DEFAULT, iree_allocator_system(), &instance),
		"Unable create a VM instance."
	);
	ERR_FAIL_COND_MSG(
		iree_hal_module_register_all_types(instance),
		"Unable register HAL modules."
	);
	device.catch_device(instance);
}

IREEInstance::~IREEInstance() {
	if(instance != nullptr) {iree_vm_instance_release(instance); instance = nullptr;}
	device.release_device();
}
