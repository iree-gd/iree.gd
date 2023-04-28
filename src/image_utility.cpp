#include "image_utility.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>

using namespace godot;

void ImageUtility::_bind_methods() {
    ClassDB::bind_static_method("ImageUtility", D_METHOD("bgr_to_rgb", "image"), &ImageUtility::bgr_to_rgb);
}

Ref<Image> ImageUtility::bgr_to_rgb(Ref<Image> p_source) {
    Image::UsedChannels used_channel = p_source->detect_used_channels();
    ERR_FAIL_COND_V_MSG(
        used_channel != Image::UsedChannels::USED_CHANNELS_RGB, 
        Ref<Image>(), 
        "Unable to convert image that is not using RGB channel."
    );

    const Image::Format format = Image::Format::FORMAT_RGB8; 
    Ref<Image> converted = p_source->duplicate();
    converted->convert(format);
    PackedByteArray data = converted->get_data();
    const int64_t data_size = data.size();
    ERR_FAIL_COND_V_MSG(data_size % 3 != 0, Ref<Image>(), "Unable to convert corrupted image.");

    PackedByteArray new_data;
    new_data.resize(data_size);
    for(int64_t i = 0; i < data_size; i += 3) {
        const uint8_t b = data[i];
        const uint8_t g = data[i + 1];
        const uint8_t r = data[i + 2];
        new_data[i] = r;
        new_data[i + 1] = g;
        new_data[i + 2] = b;
    }

    converted->set_data(
        converted->get_width(),
        converted->get_height(),
        false, format, new_data
    );

    return converted;
}