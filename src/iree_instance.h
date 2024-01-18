#ifndef IREE_INSTANCE_H
#define IREE_INSTANCE_H

#include <iree/base/api.h>
#include <iree/hal/api.h>
#include <iree/vm/api.h>

#include "iree_device.h"

namespace godot
{

    class IREEInstance
    {
    private:
        static IREEInstance *singleton;

        iree_vm_instance_t *vm_instance;
        IREEDevice device;

        IREEInstance();
        ~IREEInstance();

        Error assure_vm_instance_captured();
        Error assure_device_captured();

    public:
        static IREEInstance *borrow_singleton();

        Error capture();
        void release();
        iree_vm_instance_t *borrow_assured_vm_instance();
        iree_hal_device_t *borrow_assured_hal_device();
        iree_vm_module_t *borrow_assured_hal_module();

        bool is_valid() const;
    };

} // namespace godot

#endif // IREE_INSTANCE_H
