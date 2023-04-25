#include "resource_format_loader_iree_module.h"

#include "iree_module.h"

using namespace godot;

Ref<Resource> ResourceFormatLoaderIREEModule::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
    Ref<IREEModule> m;
    m.instantiate();
    Error e = m->load(p_path);
    if(r_error) *r_error = e;
    if(e != OK) return Ref<Resource>();
    return m;
}


void ResourceFormatLoaderIREEModule::get_recognized_extensions(List<String> *p_extensions) const {
    p_extensions->push_back("vmfb");
}

bool ResourceFormatLoaderIREEModule::handles_type(const String &p_type) const {
    return p_type == "IREEModule";
}

String ResourceFormatLoaderIREEModule::get_resource_type(const String &p_path) const {
    if(p_path.get_extension().to_lower() == "vmfb") return "IREEModule";
    return "";
}