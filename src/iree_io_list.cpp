#include "iree_io_list.h"

#include <iree/modules/hal/module.h>
#include <godot_cpp/core/error_macros.hpp>

#include "iree_buffer_view.h"

#define INIT_CAPACITY 10

using namespace godot;

bool IREEIOList::is_null() const {
    return list != nullptr;
}

Error IREEIOList::init() {
    if(list != nullptr) return OK;
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

iree_vm_list_t* IREEIOList::get_assign_raw_list() {
    return list;
}

Array IREEIOList::to_array() const {
    if(list == nullptr) return Array();
    Array result;
    iree_host_size_t size = iree_vm_list_size(list);
    for(iree_host_size_t i = 0; i < size; i++) {
        iree_hal_buffer_view_t* buffer_view = iree_vm_list_get_buffer_view_retain(list, i);
        Ref<IREEBufferView> buffer_view_ref;
        buffer_view_ref.instantiate();
        buffer_view_ref->set_move_raw_buffer_view(buffer_view);
        result.append(buffer_view_ref);
    }
    return result;
}

Error IREEIOList::append_retain_raw_buffer_view(iree_hal_buffer_view_t* m_buffer_view) {
    if(m_buffer_view == nullptr) return ERR_INVALID_PARAMETER;
    iree_vm_ref_t buffer_view_ref = iree_hal_buffer_view_retain_ref(m_buffer_view);
    if(iree_vm_list_push_ref_move(list, &buffer_view_ref)) {
        ERR_PRINT("Unable to append IREE buffer view to IREE list.");
        iree_vm_ref_release(&buffer_view_ref);
        return FAILED;
    }
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