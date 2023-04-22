#include "iree_io_list.h"

#include <iree/modules/hal/module.h>
#include <godot_cpp/core/error_macros.hpp>

#include "iree_buffer_view.h"

#define INIT_CAPACITY 10

using namespace godot;

Array IREEIOList::to_array(const iree_vm_list_t* p_list) {
    if(p_list == nullptr) return Array();
    Array result;
    iree_host_size_t size = iree_vm_list_size(p_list);
    for(iree_host_size_t i = 0; i < size; i++) {
        iree_hal_buffer_view_t* buffer_view = iree_vm_list_get_buffer_view_assign(p_list, i);
        iree_hal_buffer_view_retain(buffer_view);
        Ref<IREEBufferView> buffer_view_ref;
        buffer_view_ref.instantiate();
        buffer_view_ref->set_raw_buffer_view(buffer_view);
        result.append(buffer_view_ref);
    }
    return result;
}

bool IREEIOList::is_init() const {
    return list != nullptr;
}

Error IREEIOList::init() {
    iree_vm_list_t* new_list = nullptr;
    ERR_FAIL_COND_V_MSG(iree_vm_list_create(
        iree_vm_make_undefined_type_def(),
        INIT_CAPACITY, iree_allocator_system(), &new_list
    ), ERR_CANT_CREATE, "Unable to create a list.");
    list = new_list;
    return OK;
}

void IREEIOList::deinit() {
    if(list != nullptr) {
        iree_vm_list_release(list);
        list = nullptr;
    }
}

const iree_vm_list_t* IREEIOList::ptr() const {
    return list;
}

iree_vm_list_t* IREEIOList::ptrw() {
    return list;
}

Array IREEIOList::to_array() const {
    return to_array(list);
}

Error IREEIOList::append(iree_hal_buffer_view_t* p_buffer_view) {
    iree_vm_ref_t buffer_view_ref = iree_hal_buffer_view_move_ref(p_buffer_view);
    ERR_FAIL_COND_V_MSG(iree_vm_list_push_ref_move(list, &buffer_view_ref), FAILED, "Unable to append IREE buffer view to IREE list.");
    return OK;
}

Error IREEIOList::append(const iree_hal_buffer_view_t* p_buffer_view) {
    iree_vm_ref_t buffer_view_ref = iree_hal_buffer_view_retain_ref((iree_hal_buffer_view_t*)p_buffer_view);
    ERR_FAIL_COND_V_MSG(iree_vm_list_push_ref_move(list, &buffer_view_ref), FAILED, "Unable to append IREE buffer view to IREE list.");
    return OK;
}

IREEIOList::IREEIOList(IREEIOList&& p_list)
:
    list(p_list.list)
{
    p_list.list = nullptr;
}

IREEIOList::IREEIOList() 
: 
    list(nullptr)
{ }

IREEIOList::~IREEIOList() { deinit(); }