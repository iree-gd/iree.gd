#include "iree.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/global_constants.hpp>

#include <iree/modules/hal/types.h>

using namespace godot;

IREE* IREE::singleton = nullptr;

iree_vm_instance_t *IREE::get_vm_instance() {
    if(IREE::singleton == nullptr)
        IREE::singleton = new IREE();

    return IREE::singleton->instance;
}

iree_hal_device_t* IREE::get_hal_device() {
    if(IREE::singleton == nullptr)
        IREE::singleton = new IREE();

    return IREE::singleton->device.device;
}

iree_vm_module_t* IREE::get_hal_module() {
    if(IREE::singleton == nullptr)
        IREE::singleton = new IREE();

    return IREE::singleton->device.hal_module;
}

IREE::IREE() 
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
	device.capture(instance);
}

IREE::~IREE() {
	if(instance != nullptr) {iree_vm_instance_release(instance); instance = nullptr;}
	device.release();
}
