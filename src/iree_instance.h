#ifndef IREE_INSTANCE_H
#define IREE_INSTANCE_H

#include <iree/base/api.h>
#include <iree/hal/api.h>
#include <iree/vm/api.h>

class IREEInstance {
private:
    static IREEInstance* singleton;

    iree_vm_instance_t* instance;

public:
    static iree_vm_instance_t* get_vm_instance();

    IREEInstance();
    ~IREEInstance();
};

#endif //IREE_INSTANCE_H