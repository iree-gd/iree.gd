#ifndef IMAGE_UTILITY_H
#define IMAGE_UTILITY_H

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/object.hpp>

namespace godot {

/* Static methods for image manipulation. */
class ImageUtility : public Object {
    GDCLASS(ImageUtility, Object)

protected:
    static void _bind_methods();

public:
    static Ref<Image> bgr_to_rgb(Ref<Image> p_source);
};

}; // namespace godot

#endif//IMAGE_UTILITY_H