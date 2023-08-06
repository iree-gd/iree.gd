#include "iree_instance.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/global_constants.hpp>

#include <iree/modules/hal/types.h>

using namespace godot;

IREEInstance* IREEInstance::singleton = nullptr;

IREEInstance::IREEInstance() 
:
    vm_instance(nullptr),
	device()
{
    capture();
}

IREEInstance::~IREEInstance() {
    release();
}

Error IREEInstance::capture() {
	ERR_FAIL_COND_V_MSG(
		iree_vm_instance_create(IREE_VM_TYPE_CAPACITY_DEFAULT, iree_allocator_system(), &vm_instance),
        ERR_CANT_CREATE,
		"Unable create a VM instance."
	);
	ERR_FAIL_COND_V_MSG(
		iree_hal_module_register_all_types(vm_instance),
        ERR_CANT_CREATE,
		"Unable register HAL modules."
	);

    // TODO: add fallbacks.
	device.capture_vulkan(vm_instance);

    return OK;
}

void IREEInstance::release() {
	if(vm_instance != nullptr) {iree_vm_instance_release(vm_instance); vm_instance = nullptr;}
	device.release();
}

IREEInstance* IREEInstance::borrow_singleton() {
    if(IREEInstance::singleton == nullptr) IREEInstance::singleton = new IREEInstance();
    return IREEInstance::singleton;
}

bool IREEInstance::is_valid() const {
    return vm_instance != nullptr && device.is_valid();
}

iree_vm_instance_t* IREEInstance::borrow_vm_instance() const {
    return vm_instance;
}

iree_hal_device_t* IREEInstance::borrow_hal_device() const {
    return device.borrow_hal_device();
}

iree_vm_module_t* IREEInstance::borrow_hal_module() const {
    return device.borrow_hal_module();
}

