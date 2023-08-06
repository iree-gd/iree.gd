#include "iree_list.h"

#include <iree/modules/hal/module.h>
#include <godot_cpp/core/error_macros.hpp>

#include "iree_tensor.h"

using namespace godot;

IREEList::IREEList()
:
    vm_list(nullptr)
{}

IREEList::IREEList(IREEList& p_list) 
:
    vm_list(p_list.share_vm_list())
{}

IREEList::IREEList(IREEList&& p_list)
:
    vm_list(p_list.give_vm_list())
{}

IREEList::~IREEList() {
    release();
}

Error IREEList::append(IREETensor& p_tensor) {
    if(p_tensor.is_null()) return ERR_INVALID_PARAMETER;
    iree_hal_buffer_view_t* buffer_view = p_tensor.share_buffer_view();
    iree_vm_ref_t ref = iree_hal_buffer_view_move_ref(buffer_view);
    if(iree_vm_list_push_ref_move(vm_list, &ref)) {
        ERR_PRINT("Unable to append IREE Tensor to IREE List.");
        iree_vm_ref_release(&ref);
        return FAILED;
    }
    return OK;
}

Error IREEList::append(IREETensor&& p_tensor) {
    if(p_tensor.is_null()) return ERR_INVALID_PARAMETER;
    iree_hal_buffer_view_t* buffer_view = p_tensor.give_buffer_view();
    iree_vm_ref_t ref = iree_hal_buffer_view_move_ref(buffer_view);
    if(iree_vm_list_push_ref_move(vm_list, &ref)) {
        ERR_PRINT("Unable to append IREE Tensor to IREE List.");
        iree_vm_ref_release(&ref);
        return FAILED;
    }
    return OK;
}

iree_vm_list_t* IREEList::give_vm_list() {
    iree_vm_list_t* returning_vm_list = vm_list;
    vm_list = nullptr;
    return returning_vm_list;
}

iree_vm_list_t* IREEList::share_vm_list() {
    iree_vm_list_retain(vm_list);
    return vm_list;
}

Error IREEList::capture(iree_host_size_t p_initial_capacity) {
    release();
    iree_vm_list_t* new_vm_list = nullptr;
    ERR_FAIL_COND_V_MSG(iree_vm_list_create(
        iree_vm_make_undefined_type_def(),
        p_initial_capacity, iree_allocator_system(), &new_vm_list
    ), ERR_CANT_CREATE, "Unable to create an IREE list.");
    vm_list = new_vm_list;
    return OK;
}

void IREEList::release() {
    if(vm_list != nullptr) { iree_vm_list_release(vm_list); vm_list = nullptr; }
}

iree_vm_list_t* IREEList::borrow_vm_list() const {
    return vm_list;
}

bool IREEList::is_null() const {
    return vm_list == nullptr;
}

Array IREEList::get_tensors() const {
    if(is_null()) return Array();
    Array tensors;
    iree_host_size_t size = iree_vm_list_size(vm_list);
    for(iree_host_size_t i = 0; i < size; i++) {
        iree_hal_buffer_view_t* buffer_view = iree_vm_list_get_buffer_view_retain(vm_list, i);
        Ref<IREETensor> tensor_ref;
        tensor_ref.instantiate();
        tensor_ref->set_buffer_view(buffer_view);
        tensors.append(tensor_ref);
    }
    return tensors;
}
