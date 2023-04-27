#include "iree_device.h"

#include <iree/hal/drivers/local_task/task_device.h>
#include <iree/hal/local/executable_loader.h>
#include <iree/hal/local/loaders/embedded_elf_loader.h>
#include <iree/hal/drivers/local_sync/sync_device.h>
#include <iree/hal/drivers/vulkan/registration/driver_module.h>
#include <iree/hal/local/loaders/vmvx_module_loader.h>
#include <iree/task/api.h>
#include <iree/modules/hal/module.h>

#include <godot_cpp/core/error_macros.hpp>

#define IREE_MAX_EXECUTOR_COUNT 8

using namespace godot;

Error IREEDevice::capture_vmvx(iree_vm_instance_t* p_instance) {
    if(p_instance == nullptr) return ERR_INVALID_PARAMETER;

    release();

    Error e = OK;
    iree_string_view_t id = iree_make_cstring_view("vmvx");
    iree_hal_sync_device_params_t params;
    iree_hal_executable_loader_t* loader = nullptr;
    iree_hal_allocator_t* device_allocator = nullptr;
    iree_hal_device_t* new_device = nullptr;
    iree_vm_module_t* new_hal_module = nullptr;
    iree_hal_sync_device_params_initialize(&params);
    
    // Create loader.
    ERR_FAIL_COND_V_MSG(iree_hal_vmvx_module_loader_create(
        p_instance, /*user_module_count=*/0, /*user_module=*/NULL,
        iree_allocator_system(), &loader
    ), ERR_CANT_CREATE, "Unable to create loader for CPU task.");

    // Create device allocator.
    if(iree_hal_allocator_create_heap(
        id, iree_allocator_system(),
        iree_allocator_system(), &device_allocator
    )) {
        e = ERR_CANT_CREATE;
        ERR_PRINT("Unable to create VMVX device allocator.");
        goto clean_up_loader;
    }

    // Create the device.
    if(iree_hal_sync_device_create(
        id, &params, /*loader_count=*/1, &loader, device_allocator,
        iree_allocator_system(), &new_device
    )) {
        e = ERR_CANT_CREATE;
        ERR_PRINT("Unable to create VMVX device.");
        goto clean_up_device_allocator;
    }

    // Create hal module.
    if(iree_hal_module_create(
        p_instance, new_device, IREE_HAL_MODULE_FLAG_SYNCHRONOUS,
        iree_allocator_system(), &new_hal_module
    )) {
        e = ERR_CANT_CREATE;
        ERR_PRINT("Unable to create HAL module of the device.");
        goto clean_up_device;
    }

    // Setup value.
    device = new_device;
    hal_module = new_hal_module;

    goto clean_up_device_allocator;

clean_up_device:
    iree_hal_device_release(new_device);

clean_up_device_allocator:
    iree_hal_allocator_release(device_allocator);

clean_up_loader:
    iree_hal_executable_loader_release(loader);
    
    return e;
}

Error IREEDevice::capture_vulkan(iree_vm_instance_t* p_instance) {
    if(p_instance == nullptr) return ERR_INVALID_PARAMETER;

    release();

    ERR_FAIL_COND_V_MSG(
        iree_hal_vulkan_driver_module_register(iree_hal_driver_registry_default()),
        FAILED, "Unable to register Vulkan HAL driver."
    );

    Error error = OK;
    iree_string_view_t identifier = iree_make_cstring_view("vulkan");
    iree_hal_driver_t* driver = nullptr;
    iree_hal_device_t* new_device = nullptr;
    iree_vm_module_t* new_hal_module = nullptr;

    // Create driver.
    ERR_FAIL_COND_V_MSG(
        iree_hal_driver_registry_try_create(
            iree_hal_driver_registry_default(), identifier, iree_allocator_system(), &driver
        ), ERR_CANT_CREATE, "Unable to create Vulkan device."
    );

    // Create device.
    if(iree_hal_driver_create_default_device(driver, iree_allocator_system(), &new_device)) {
        ERR_PRINT("Unable to create HAL driver.");
        error = ERR_CANT_CREATE;
        goto clean_up_driver;
    }

    // Create hal module.
    if(iree_hal_module_create(
        p_instance, new_device, IREE_HAL_MODULE_FLAG_SYNCHRONOUS,
        iree_allocator_system(), &new_hal_module
    )) {
        ERR_PRINT("Unable to create HAL module of the Vulkan device.");
        error = ERR_CANT_CREATE;
        goto clean_up_device;
    }

    // Setup value.
    device = new_device;
    hal_module = new_hal_module;

    goto clean_up_driver;

clean_up_device:
    iree_hal_device_release(new_device);

clean_up_driver:
    iree_hal_driver_release(driver);

    return error;
}

Error IREEDevice::capture_cpu_async(iree_vm_instance_t* p_instance) {
    if(p_instance == nullptr) return ERR_INVALID_PARAMETER;

    release();

    Error e = OK;
    iree_string_view_t id = iree_make_cstring_view("local-task");
    iree_hal_executable_loader_t* loader = nullptr;
    iree_task_executor_t* executors[IREE_MAX_EXECUTOR_COUNT] = {0};
    iree_host_size_t executor_count = 0;
    iree_hal_allocator_t* device_allocator = nullptr;
    iree_hal_device_t* new_device = nullptr;
    iree_vm_module_t* new_hal_module = nullptr;
    iree_hal_task_device_params_t params;

    iree_hal_task_device_params_initialize(&params);

    // Create loader.
    ERR_FAIL_COND_V_MSG(iree_hal_embedded_elf_loader_create(
        /*plugin_manager=*/{0}, iree_allocator_system(), &loader
    ), ERR_CANT_CREATE, "Unable to create loader for CPU task.");

    // Create executors.
    if(iree_task_executors_create_from_flags(
        iree_allocator_system(), IREE_ARRAYSIZE(executors), executors, &executor_count
    )) {
        e = ERR_CANT_CREATE;
        ERR_PRINT("Unable to create CPU executor.");
        goto clean_up_loader;
    }

    // Create device allocator.
    if(iree_hal_allocator_create_heap(
        id, iree_allocator_system(),
        iree_allocator_system(), &device_allocator
    )) {
        e = ERR_CANT_CREATE;
        ERR_PRINT("Unable to create CPU device allocator.");
        goto clean_up_executors;
    }
    

    // Create the device.
    if(iree_hal_task_device_create(
        id, &params, executor_count, executors,
        /*loader_count=*/1, &loader, device_allocator, iree_allocator_system(),
        &new_device
    )) {
        e = ERR_CANT_CREATE;
        ERR_PRINT("Unable to capture CPU device.");
        goto clean_up_device_allocator;
    }

    // Create hal module.
    if(iree_hal_module_create(
        p_instance, new_device, IREE_HAL_MODULE_FLAG_SYNCHRONOUS,
        iree_allocator_system(), &new_hal_module
    )) {
        e = ERR_CANT_CREATE;
        ERR_PRINT("Unable to create HAL module of the device.");
        goto clean_up_device;
    }

    device = new_device;
    hal_module = new_hal_module;

    goto clean_up_device_allocator;

clean_up_device:
    iree_hal_device_release(new_device);
clean_up_device_allocator:
    iree_hal_allocator_release(device_allocator);
clean_up_executors:
    for(iree_host_size_t i = 0; i < executor_count; i++)
        iree_task_executor_release(executors[i]);
clean_up_loader:
    iree_hal_executable_loader_release(loader);

    return e;
}

void IREEDevice::release() {
    if(hal_module != nullptr) {iree_vm_module_release(hal_module); hal_module = nullptr;}
    if(device != nullptr) {iree_hal_device_release(device); device = nullptr;}
}

IREEDevice::IREEDevice()
:
    device(nullptr),
    hal_module(nullptr)
{}

IREEDevice::~IREEDevice() { release(); }