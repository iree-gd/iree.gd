#include "iree_device.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <iree/modules/hal/module.h>

#if defined(__APPLE__)
#include <iree/hal/drivers/metal/registration/driver_module.h>
#include <iree/hal/drivers/metal/api.h>
#elif defined(_WIN32) || defined(BSD) || (defined(__linux__) && !defined(__ANDROID__))
#include <iree/hal/drivers/vulkan/registration/driver_module.h>
#include <iree/hal/drivers/vulkan/api.h>
#else
#include <iree/hal/drivers/local_sync/sync_device.h>
#include <iree/hal/local/executable_loader.h>
#include <iree/hal/local/loaders/embedded_elf_loader.h>
#include <iree/hal/local/loaders/vmvx_module_loader.h>
#endif

#include "iree_error.h"

#define IREE_MAX_EXECUTOR_COUNT 8

using namespace godot;

IREEDevice::IREEDevice()
:
    hal_device(nullptr),
    hal_module(nullptr)
{}

IREEDevice::IREEDevice(IREEDevice& p_iree_device) : 
    hal_device(p_iree_device.hal_device), 
    hal_module(p_iree_device.hal_module) 
{
    iree_hal_device_retain(p_iree_device.hal_device);
    iree_vm_module_retain(p_iree_device.hal_module);
}

IREEDevice::IREEDevice(IREEDevice&& p_iree_device) : 
    hal_device(p_iree_device.hal_device), 
    hal_module(p_iree_device.hal_module) 
{
    p_iree_device.hal_device = nullptr;
    p_iree_device.hal_module = nullptr;
}

IREEDevice::~IREEDevice() {
    release();
}

Error IREEDevice::capture(iree_vm_instance_t* p_instance) {
    if(p_instance == nullptr) return ERR_INVALID_PARAMETER;

    release();

// Metal (macOS, IOS)
#if defined(__APPLE__)
    Error error = OK;
    iree_status_t iree_status = iree_ok_status();
    iree_hal_device_t* new_hal_device = nullptr;
    iree_vm_module_t* new_hal_module = nullptr;
    iree_hal_driver_t* driver = nullptr;
    iree_string_view_t identifier = iree_make_cstring_view("metal");
    
    // Register.
    IREE_ERR_V_MSG(
        iree_hal_metal_driver_module_register(iree_hal_driver_registry_default()),
        FAILED, "Unable to register Metal HAL driver."
    );

    // Create driver.
    IREE_ERR_V_MSG(
        iree_hal_driver_registry_try_create(
            iree_hal_driver_registry_default(), identifier, iree_allocator_system(), &driver
        ), ERR_CANT_CREATE, "Unable to create Metal HAL driver."
    );

    // Create device.
    if((iree_status = iree_hal_driver_create_default_device(driver, iree_allocator_system(), &new_hal_device))) {
        ERR_PRINT("Unable to create Metal HAL device.");
        ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(iree_status))));
        iree_status_fprint(stderr, iree_status);
        iree_status_free(iree_status);
        error = ERR_CANT_CREATE;
        goto clean_up_driver;
    }

    // Create HAL module.
    if((iree_status = iree_hal_module_create(
        p_instance, new_hal_device, IREE_HAL_MODULE_FLAG_SYNCHRONOUS,
        iree_allocator_system(), &new_hal_module
    ))) {
        ERR_PRINT("Unable to create HAL module of the Metal device.");
        ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(iree_status))));
        iree_status_fprint(stderr,iree_status);
        iree_status_free(iree_status);
        error = ERR_CANT_CREATE;
        goto clean_up_device;
    }

    // Setup value.
    hal_device = new_hal_device;
    hal_module = new_hal_module;    

    goto clean_up_driver;
    
    clean_up_device:
        iree_hal_device_release(new_hal_device);

    clean_up_driver:
        iree_hal_driver_release(driver);
    
    return error;

// Vulkan (Windows, Linux, *BSD)
#elif defined(_WIN32) || defined(BSD) || (defined(__linux__) && !defined(__ANDROID__))
    Error error = OK;
    iree_status_t iree_status = iree_ok_status();
    iree_hal_device_t* new_hal_device = nullptr;
    iree_vm_module_t* new_hal_module = nullptr;
    iree_string_view_t identifier = iree_make_cstring_view("vulkan");
    
    RenderingDevice* rendering_device = RenderingServer::get_singleton()->get_rendering_device();

    // TODO: In future version of godot, we would need to check whether rendering device is vulkan or not, through `get_device_capabilities`.
    if(rendering_device == nullptr) { // Not using vulkan, create vulkan device ourselves.
        iree_hal_driver_t* driver = nullptr;

        IREE_ERR_V_MSG(
            iree_hal_vulkan_driver_module_register(iree_hal_driver_registry_default()),
            FAILED, "Unable to register Vulkan HAL driver."
        );


        // Create driver.
        IREE_ERR_V_MSG(
            iree_hal_driver_registry_try_create(
                iree_hal_driver_registry_default(), identifier, iree_allocator_system(), &driver
            ), ERR_CANT_CREATE, "Unable to create Vulkan device."
        );

        // Create device.
        if((iree_status = iree_hal_driver_create_default_device(driver, iree_allocator_system(), &new_hal_device))) {
            ERR_PRINT("Unable to create Vulkan HAL device.");
            ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(iree_status))));
            iree_status_fprint(stderr,iree_status);
            iree_status_free(iree_status);
            error = ERR_CANT_CREATE;
            goto create_clean_up_driver;
        }

        // Create hal module.
        if((iree_status = iree_hal_module_create(
            p_instance, new_hal_device, IREE_HAL_MODULE_FLAG_SYNCHRONOUS,
            iree_allocator_system(), &new_hal_module
        ))) {
            ERR_PRINT("Unable to create HAL module of the Vulkan device.");
            ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(iree_status))));
            iree_status_fprint(stderr,iree_status);
            iree_status_free(iree_status);
            error = ERR_CANT_CREATE;
            goto create_clean_up_device;
        }

        // Setup value.
        hal_device = new_hal_device;
        hal_module = new_hal_module;

        goto create_clean_up_driver;

    create_clean_up_device:
        iree_hal_device_release(new_hal_device);

    create_clean_up_driver:
        iree_hal_driver_release(driver);

    } 

    else { // Godot is using vulkan, wrap vulkan.
        iree_hal_vulkan_syms_t* syms = nullptr;
        iree_hal_vulkan_driver_options_t driver_options;
        iree_hal_vulkan_queue_set_t compute_queue_set;
        iree_hal_vulkan_queue_set_t transfer_queue_set;

        const VkInstance vk_instance = (VkInstance) rendering_device->get_driver_resource(RenderingDevice::DriverResource::DRIVER_RESOURCE_VULKAN_INSTANCE, RID(), 0);
        ERR_FAIL_COND_V_MSG(vk_instance == VK_NULL_HANDLE, ERR_QUERY_FAILED, "Unable to retrieve Vulkan instance.");
        const VkPhysicalDevice vk_physical_device = (VkPhysicalDevice) rendering_device->get_driver_resource(RenderingDevice::DriverResource::DRIVER_RESOURCE_VULKAN_PHYSICAL_DEVICE, RID(), 0);
        ERR_FAIL_COND_V_MSG(vk_physical_device == VK_NULL_HANDLE, ERR_QUERY_FAILED, "Unable to retrieve Vulkan physical device.");
        const VkDevice vk_device = (VkDevice) rendering_device->get_driver_resource(RenderingDevice::DriverResource::DRIVER_RESOURCE_VULKAN_DEVICE, RID(), 0);
        ERR_FAIL_COND_V_MSG(vk_device == VK_NULL_HANDLE, ERR_QUERY_FAILED, "Unable to retrieve Vulkan device.");
        compute_queue_set.queue_family_index = rendering_device->get_driver_resource(RenderingDevice::DriverResource::DRIVER_RESOURCE_VULKAN_QUEUE_FAMILY_INDEX, RID(), 0);
        compute_queue_set.queue_indices = 1 << 0;
        transfer_queue_set.queue_indices = 0;
        driver_options.api_version = VK_API_VERSION_1_0;
        driver_options.requested_features = (iree_hal_vulkan_features_t)(IREE_HAL_VULKAN_FEATURE_ENABLE_DEBUG_UTILS);

        IREE_ERR_V_MSG(
            iree_hal_vulkan_syms_create_from_system_loader(iree_allocator_system(), &syms),
            ERR_CANT_CREATE, "Unable to create Vulkan syms."
        );

        if((iree_status = iree_hal_vulkan_wrap_device(
            identifier, &driver_options.device_options, syms, vk_instance, vk_physical_device, vk_device, &compute_queue_set,
            &transfer_queue_set, iree_allocator_system(), &new_hal_device
        ))) {
            error = ERR_CANT_CREATE;
            ERR_PRINT("Unable to wrap Vualkan device.");
            ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(iree_status))));
            iree_status_fprint(stderr,iree_status);
            iree_status_free(iree_status);
            goto wrap_clean_up_syms;
        }

        // Create hal module.
        if((iree_status = iree_hal_module_create(
            p_instance, new_hal_device, IREE_HAL_MODULE_FLAG_SYNCHRONOUS,
            iree_allocator_system(), &new_hal_module
        ))) {
            ERR_PRINT("Unable to create HAL module of the Vulkan device.");
            ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(iree_status))));
            iree_status_fprint(stderr,iree_status);
            iree_status_free(iree_status);
            error = ERR_CANT_CREATE;
            goto wrap_clean_up_device;
        }

        // Setup value.
        hal_device = new_hal_device;
        hal_module = new_hal_module;


        wrap_clean_up_device:
            iree_hal_device_release(new_hal_device);

        wrap_clean_up_syms: 
            iree_hal_vulkan_syms_release(syms);
    }

    return error;

// VMVX (Android, Web)
#else
    Error error = OK;
    iree_status_t status = iree_ok_status();
    iree_string_view_t id = iree_make_cstring_view("vmvx");
    iree_hal_sync_device_params_t params;
    iree_hal_executable_loader_t* loader = nullptr;
    iree_hal_allocator_t* device_allocator = nullptr;
    iree_hal_device_t* new_hal_device = nullptr;
    iree_vm_module_t* new_hal_module = nullptr;
    iree_hal_sync_device_params_initialize(&params);
    
    // Create loader.
    IREE_ERR_V_MSG(iree_hal_vmvx_module_loader_create(
        p_instance, /*user_module_count=*/0, /*user_module=*/NULL,
        iree_allocator_system(), &loader
    ), ERR_CANT_CREATE, "Unable to create loader for CPU task.");

    // Create device allocator.
    if((status = iree_hal_allocator_create_heap(
        id, iree_allocator_system(),
        iree_allocator_system(), &device_allocator
    ))) {
        error = ERR_CANT_CREATE;
        ERR_PRINT("Unable to create VMVX device allocator.");
        ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(status))));
        iree_status_fprint(stderr, status);
        iree_status_free(status);
        goto clean_up_loader;
    }

    // Create the device.
    if((status = iree_hal_sync_device_create(
        id, &params, /*loader_count=*/1, &loader, device_allocator,
        iree_allocator_system(), &new_hal_device
    ))) {
        error = ERR_CANT_CREATE;
        ERR_PRINT("Unable to create VMVX device.");
        ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(status))));
        iree_status_fprint(stderr, status);
        iree_status_free(status);
        goto clean_up_device_allocator;
    }

    // Create hal module.
    if((status = iree_hal_module_create(
        p_instance, new_hal_device, IREE_HAL_MODULE_FLAG_SYNCHRONOUS,
        iree_allocator_system(), &new_hal_module
    ))) {
        error = ERR_CANT_CREATE;
        ERR_PRINT("Unable to create HAL module of the device.");
        ERR_PRINT(vformat("IREE code: '%s'. IREE's error is logged to `stderr`.", iree_status_code_string(iree_status_code(status))));
        iree_status_fprint(stderr, status);
        iree_status_free(status);
        goto clean_up_device;
    }

    // Setup value.
    hal_device = new_hal_device;
    hal_module = new_hal_module;

    goto clean_up_device_allocator;

clean_up_device:
    iree_hal_device_release(new_hal_device);

clean_up_device_allocator:
    iree_hal_allocator_release(device_allocator);

clean_up_loader:
    iree_hal_executable_loader_release(loader);
    
    return error;
#endif
}

void IREEDevice::release() {
    if(hal_device != nullptr) {iree_hal_device_release(hal_device); hal_device = nullptr;}
    if(hal_module != nullptr) {iree_vm_module_release(hal_module); hal_module = nullptr;}
}

bool IREEDevice::is_valid() const {
    return hal_device != nullptr && hal_module != nullptr;
}

iree_hal_device_t* IREEDevice::borrow_hal_device() const {
    return hal_device;
}

iree_vm_module_t* IREEDevice::borrow_hal_module() const {
    return hal_module;
}
