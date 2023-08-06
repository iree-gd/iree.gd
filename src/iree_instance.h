#ifndef IREE_INSTANCE_H
#define IREE_INSTANCE_H

#include <iree/base/api.h>
#include <iree/hal/api.h>
#include <iree/vm/api.h>

#include "iree_device.h"

class IREEInstance {
private:
    static IREEInstance* singleton;

    iree_vm_instance_t* vm_instance;
    IREEDevice device;

    IREEInstance();
    ~IREEInstance();
public:
    static IREEInstance* borrow_singleton();

    Error capture();
    void release();

    bool is_valid() const;
    iree_vm_instance_t* borrow_vm_instance() const;
    iree_hal_device_t* borrow_hal_device() const;
    iree_vm_module_t* borrow_hal_module() const;
};

#endif //IREE_INSTANCE_H
