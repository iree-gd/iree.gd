#ifndef RESOURCE_FORMAT_LOADER_IREE_MODULE_H
#define RESOURCE_FORMAT_LOADER_IREE_MODULE_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/resource_format_loader.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class ResourceFormatLoaderIREEModule : public ResourceFormatLoader {

public: 
	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

} // godot

#endif//RESOURCE_FORMAT_LOADER_IREE_MODULE_H