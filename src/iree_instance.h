#ifndef IREE_INSTANCE_H
#define IREE_INSTANCE_H

#include <iree/base/api.h>
#include <iree/hal/api.h>
#include <iree/vm/api.h>

#include "iree_device.h"

class IREEInstance {
private:
    static IREEInstance* singleton;

    iree_vm_instance_t* instance;
    IREEDevice device;

    IREEInstance();
    ~IREEInstance();
public:
    static iree_vm_instance_t* get_vm_instance();
    static iree_hal_device_t* get_device();
    static iree_vm_module_t* get_hal_module();
};

#endif //IREE_INSTANCE_H