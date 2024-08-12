#ifndef IREE_LIST_H
#define IREE_LIST_H

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include <iree/hal/api.h>
#include <iree/vm/api.h>

#include "iree_tensor.h"

namespace godot {

class IREEList {
private:
	iree_vm_list_t *vm_list;

public:
	IREEList();
	IREEList(IREEList &p_list);
	IREEList(IREEList &&p_list);
	~IREEList();

	Error append(IREETensor &p_tensor);
	Error append(IREETensor &&p_tensor);
	iree_vm_list_t *give_vm_list();
	iree_vm_list_t *share_vm_list();
	Error capture(iree_host_size_t p_initial_capacity);
	void release();

	iree_vm_list_t *borrow_vm_list() const;
	bool is_captured() const;
	TypedArray<IREETensor> get_tensors() const;
};

} // namespace godot

#endif // IREE_LIST_H
