#ifndef IREE_IO_LIST_H
#define IREE_IO_LIST_H

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <iree/vm/api.h>
#include <iree/hal/api.h>

#include "iree_buffer_view.h"

namespace godot {

class IREEIOList {
private:
    iree_vm_list_t* list;
public:
    IREEIOList();
    IREEIOList(IREEIOList&& p_list);
    ~IREEIOList();

    bool is_null() const;
    Error init();
    void deinit();

    iree_vm_list_t* get_assign_raw_list();

    Array to_array() const;
    
    Error append_retain_raw_buffer_view(iree_hal_buffer_view_t* m_buffer_view);
};

} // namespace godot

#endif//IREE_IO_LIST_H