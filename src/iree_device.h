#ifndef IREE_DEVICE_H
#define IREE_DEVICE_H

#include <iree/base/api.h>
#include <iree/hal/api.h>
#include <iree/vm/api.h>

#include <godot_cpp/classes/global_constants.hpp>

namespace godot {

/* Simple structure that yield and manage device information. */
class IREEDevice {
private:
	iree_hal_device_t *hal_device;
	iree_vm_module_t *hal_module;

public:
	IREEDevice();
	IREEDevice(IREEDevice &p_iree_device);
	IREEDevice(IREEDevice &&p_iree_device);
	~IREEDevice();

	Error capture(iree_vm_instance_t *p_instance);
	void release();

	bool is_captured() const;
	iree_hal_device_t *borrow_hal_device() const;
	iree_vm_module_t *borrow_hal_module() const;
};

} // namespace godot

#endif // IREE_DEVICE_H
