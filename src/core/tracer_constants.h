#ifndef TRACER_CONSTANTS_H
#define TRACER_CONSTANTS_H

namespace tracer_constants
{
    constexpr auto aspect_ratio = 4.0 / 3.0;
    constexpr int image_width = 720;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);
    constexpr int color_channels = 3;
    constexpr int frame_size = image_width * image_height * color_channels;
    constexpr bool stack_alloc = true;
    constexpr int samples_per_pixel = 100;
    constexpr int max_depth = 50;
    constexpr bool progress_gui = true;
}

#endif