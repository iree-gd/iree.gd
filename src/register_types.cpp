#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

#include "iree_tensor.h"
#include "iree_module.h"
#include "resource_format_loader_iree_module.h"

using namespace godot;

static Ref<ResourceFormatLoaderIREEModule> resource_loader_iree_module;

static void initialize_iree_gd_module(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		return;
	}

	GDREGISTER_CLASS(IREEModule);
	GDREGISTER_CLASS(IREETensor);
	GDREGISTER_CLASS(ResourceFormatLoaderIREEModule);
	resource_loader_iree_module.instantiate();
	ResourceLoader::get_singleton()->add_resource_format_loader(resource_loader_iree_module);
}

static void uninitialize_iree_gd_module(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		return;
	}

	ResourceLoader::get_singleton()->remove_resource_format_loader(resource_loader_iree_module);
	resource_loader_iree_module.unref();
}

extern "C"
{
	// Initialization.
	GDExtensionBool GDE_EXPORT iree_gd_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(initialize_iree_gd_module);
		init_obj.register_terminator(uninitialize_iree_gd_module);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}
