#ifndef IREE_IO_LIST_H
#define IREE_IO_LIST_H

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/global_constants.hpp>

#include <iree/vm/api.h>
#include <iree/hal/api.h>

namespace godot {

class IREEIOList {
private:
    iree_vm_list_t* list;
public:
    static Array create_arrays_from_raw_list(const iree_vm_list_t* p_list);

    IREEIOList(IREEIOList&& p_list);
    IREEIOList();
    ~IREEIOList();

    bool is_init() const;
    Error init();
    void deinit();

    const iree_vm_list_t* ptr() const;
    iree_vm_list_t* ptrw();

    Array create_arrays() const;

    Error append_move(iree_hal_buffer_view_t* p_buffer_view);
};

} // namespace godot

#endif//IREE_IO_LIST_H