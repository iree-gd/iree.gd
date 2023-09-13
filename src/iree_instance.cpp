#include "iree_instance.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/global_constants.hpp>

#include <iree/modules/hal/types.h>

#include "iree_error.h"

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

Error IREEInstance::assure_vm_instance_captured() {
    if(vm_instance != nullptr) return OK;


	IREE_ERR_V_MSG(
		iree_vm_instance_create(IREE_VM_TYPE_CAPACITY_DEFAULT, iree_allocator_system(), &vm_instance),
        ERR_CANT_CREATE,
		"Unable create a VM instance."
	);

	IREE_ERR_V_MSG(
		iree_hal_module_register_all_types(vm_instance),
        ERR_CANT_CREATE,
		"Unable register HAL modules."
	);
    return OK;
}

Error IREEInstance::assure_device_captured() {
    Error error = OK;

    error = assure_vm_instance_captured();
    ERR_FAIL_COND_V(error != OK, error);

    // TODO: add fallbacks.
	error = device.capture_vulkan(vm_instance);
    ERR_FAIL_COND_V(error != OK, error);
    return OK;
}

Error IREEInstance::capture() {
    Error error = OK;

    error = assure_vm_instance_captured();
    ERR_FAIL_COND_V(error != OK, error);
	error = assure_device_captured();
    ERR_FAIL_COND_V(error != OK, error);
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

iree_vm_instance_t* IREEInstance::borrow_assured_vm_instance() {
    Error error = assure_vm_instance_captured();
    ERR_FAIL_COND_V(error != OK, nullptr);
    return vm_instance;
}

iree_hal_device_t* IREEInstance::borrow_assured_hal_device() {
    Error error = assure_device_captured();
    ERR_FAIL_COND_V(error != OK, nullptr);
    return device.borrow_hal_device();
}

iree_vm_module_t* IREEInstance::borrow_assured_hal_module() {
    Error error = assure_device_captured();
    ERR_FAIL_COND_V(error != OK, nullptr);
    return device.borrow_hal_module();
}

