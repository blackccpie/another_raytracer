#ifndef TRACER_CONSTANTS_H
#define TRACER_CONSTANTS_H

namespace tracer_constants
{
    constexpr auto aspect_ratio = 4.0 / 3.0;
    constexpr int image_width = 720;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);
}

#endif