#include "iree_tensor.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include "iree_instance.h"

#define MAX_SHAPE_RANK 50
#define STRINGIFY_MACROS(m_macros) #m_macros

using namespace godot;

template<iree_hal_element_type_t T>
Error IREETensor::capture(typename StorageType<T>::type p_data, PackedInt64Array p_dimension) {
    release();
    const int64_t shape_rank = p_dimension.size();
    ERR_FAIL_COND_V_MSG(shape_rank == 0, ERR_PARAMETER_RANGE_ERROR, "Dimensionless tensor is forbidden.");
    ERR_FAIL_COND_V_MSG(shape_rank > MAX_SHAPE_RANK, ERR_PARAMETER_RANGE_ERROR, "Dimension exceed maximum rank (" STRINGIFY_MACROS(MAX_SHAPE_RANK) ").");

    int64_t expected_size = 1;
    for(int64_t i = 0; i < shape_rank; i++) expected_size *= p_dimension[i];
    ERR_FAIL_COND_V_MSG(expected_size != p_data.size(), ERR_INVALID_PARAMETER, "Dimension doesn't match with the data.");

    iree_hal_dim_t shape[MAX_SHAPE_RANK] = {0};
    for(int i = 0; i < shape_rank; i++) shape[i] = p_dimension[i];

    iree_hal_device_t* const device = IREEInstance::borrow_singleton()->borrow_hal_device();
    ERR_FAIL_COND_V(device == nullptr, ERR_QUERY_FAILED);

    iree_hal_buffer_params_t buffer_params = {0};
    buffer_params.usage = IREE_HAL_BUFFER_USAGE_DEFAULT;
    buffer_params.type = IREE_HAL_MEMORY_TYPE_DEVICE_LOCAL;
    
    ERR_FAIL_COND_V_MSG(
        iree_hal_buffer_view_allocate_buffer_copy(
            device,
            iree_hal_device_allocator(device), shape_rank, shape, T, 
            IREE_HAL_ENCODING_TYPE_DENSE_ROW_MAJOR, buffer_params,
            iree_make_const_byte_span(p_data.ptr(), p_data.size() * StorageType<T>::element_size), &buffer_view
    ), ERR_CANT_CREATE, "Unable to allocate buffer on device for IREE tensor.");
    return OK;
}

template<iree_hal_element_type_t T>
Ref<IREETensor> IREETensor::from(typename StorageType<T>::type p_data, PackedInt64Array p_dimension) {
    Ref<IREETensor> tensor_ref;
    tensor_ref.instantiate();
    tensor_ref->capture<T>(p_data, p_dimension);
    return tensor_ref;
}

void IREETensor::_bind_methods() {
    ClassDB::bind_static_method("IREETensor", D_METHOD("from_bytes", "bytes", "dimension"), &IREETensor::from<IREE_HAL_ELEMENT_TYPE_UINT_8>);
    ClassDB::bind_static_method("IREETensor", D_METHOD("from_float32s", "float32s", "dimension"), &IREETensor::from<IREE_HAL_ELEMENT_TYPE_FLOAT_32>);
    ClassDB::bind_static_method("IREETensor", D_METHOD("from_float64s", "float64s", "dimension"), &IREETensor::from<IREE_HAL_ELEMENT_TYPE_FLOAT_64>);
    ClassDB::bind_static_method("IREETensor", D_METHOD("from_int32s", "int32s", "dimension"), &IREETensor::from<IREE_HAL_ELEMENT_TYPE_SINT_32>);
    ClassDB::bind_static_method("IREETensor", D_METHOD("from_int64s", "int64s", "dimension"), &IREETensor::from<IREE_HAL_ELEMENT_TYPE_SINT_64>);

    ClassDB::bind_method(D_METHOD("capture_bytes", "bytes", "dimension"), &IREETensor::capture<IREE_HAL_ELEMENT_TYPE_UINT_8>);
    ClassDB::bind_method(D_METHOD("capture_float32s", "float32s", "dimension"), &IREETensor::capture<IREE_HAL_ELEMENT_TYPE_FLOAT_32>);
    ClassDB::bind_method(D_METHOD("capture_float64s", "float64s", "dimension"), &IREETensor::capture<IREE_HAL_ELEMENT_TYPE_FLOAT_64>);
    ClassDB::bind_method(D_METHOD("capture_int32s", "int32s", "dimension"), &IREETensor::capture<IREE_HAL_ELEMENT_TYPE_SINT_32>);
    ClassDB::bind_method(D_METHOD("capture_int64s", "int64s", "dimension"), &IREETensor::capture<IREE_HAL_ELEMENT_TYPE_SINT_64>);
    ClassDB::bind_method(D_METHOD("release"), &IREETensor::release);
    ClassDB::bind_method(D_METHOD("is_null"), &IREETensor::is_null);
    ClassDB::bind_method(D_METHOD("get_data"), &IREETensor::get_data);
    ClassDB::bind_method(D_METHOD("get_dimension"), &IREETensor::get_dimension);
}

IREETensor::IREETensor()
:
    buffer_view(nullptr)
{}

IREETensor::IREETensor(IREETensor& p_tensor)
:
    buffer_view(p_tensor.share_buffer_view())
{}

IREETensor::IREETensor(IREETensor&& p_tensor)
:
    buffer_view(p_tensor.give_buffer_view())
{}

IREETensor::~IREETensor() {
    release();    
}

void IREETensor::set_buffer_view(iree_hal_buffer_view_t *p_buffer_view) {
    buffer_view = p_buffer_view;
}

iree_hal_buffer_view_t* IREETensor::give_buffer_view() {
    iree_hal_buffer_view_t* returning_buffer_view = buffer_view;
    buffer_view = nullptr;
    return returning_buffer_view;
}

iree_hal_buffer_view_t* IREETensor::share_buffer_view() {
    iree_hal_buffer_view_retain(buffer_view);
    return buffer_view;
}

void IREETensor::release() {
    if(buffer_view != nullptr) { iree_hal_buffer_view_release(buffer_view); buffer_view = nullptr; }
}

bool IREETensor::is_null() const {
    return buffer_view == nullptr;
}

iree_hal_buffer_view_t* IREETensor::borrow_buffer_view() const {
    return buffer_view;
}

PackedByteArray IREETensor::get_data() const {
    if(buffer_view == nullptr) return PackedByteArray();
    PackedByteArray data;

    iree_device_size_t data_size = iree_hal_buffer_view_byte_length(buffer_view);
    data.resize(data_size);
    ERR_FAIL_COND_V_MSG(data.size() != data_size, PackedByteArray(), "Fail to allocate memory for data in IREE Tensor for retrieving data.");

    ERR_FAIL_COND_V_MSG(
        iree_hal_device_transfer_d2h(
            IREEInstance::borrow_singleton()->borrow_hal_device(), iree_hal_buffer_view_buffer(buffer_view),
            0, data.ptrw(), data_size, IREE_HAL_TRANSFER_BUFFER_FLAG_DEFAULT, iree_infinite_timeout()
    ), PackedByteArray(), "Unable to retrieve data from device in IREE Tensor.");

    return data;
}

Array IREETensor::get_dimension() const {
    iree_host_size_t shape_rank = 0;
    iree_hal_dim_t shape[MAX_SHAPE_RANK] = {0};
    ERR_FAIL_COND_V_MSG(iree_hal_buffer_view_shape(buffer_view, MAX_SHAPE_RANK, shape, &shape_rank),
            Array(), "Dimension of IREETensor exceed maximum rank (" STRINGIFY_MACROS(MAX_SHAPE_RANK) ") to be acquired.");
    Array dimension;
    for(iree_host_size_t i = 0; i < shape_rank; i++) dimension.push_back(shape[i]);
    return dimension;
}
