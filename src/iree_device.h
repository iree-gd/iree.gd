#ifndef IREE_DEVICE_H
#define IREE_DEVICE_H

#include <iree/base/api.h>
#include <iree/hal/api.h>
#include <iree/vm/api.h>

#include <godot_cpp/classes/global_constants.hpp>

using namespace godot;

/* Simple structure that holds device information. */
class IREEDevice {
private:
    IREEDevice(const IREEDevice& p_iree_device) = delete; // no copy or move
public:
    iree_hal_device_t* device;
    iree_vm_module_t* hal_module;

    IREEDevice();
    ~IREEDevice();

    Error capture_vmvx(iree_vm_instance_t* p_instance);
    Error capture_cpu_async(iree_vm_instance_t* p_instance);
    Error capture_vulkan(iree_vm_instance_t* p_instance);
    void release();
};

#endif // IREE_DEVICE_H