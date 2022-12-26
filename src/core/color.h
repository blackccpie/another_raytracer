#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

template<typename T = std::uint8_t>
inline void write_color(T* out, color pixel_color, int samples_per_pixel) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Divide the color by the number of samples and gamma-correct for gamma=2.0.
    auto scale = 1.0 / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);
    
    // Write the translated [0,255] value of each color component.
    out[0] = static_cast<T>(256 * clamp(r, 0.0, 0.999));
    out[1] = static_cast<T>(256 * clamp(g, 0.0, 0.999));
    out[2] = static_cast<T>(256 * clamp(b, 0.0, 0.999));
}

template<typename T = std::uint8_t>
inline void write_color_raw(T* out, color pixel_color)
{
    out[0] = static_cast<T>(pixel_color.x());
    out[1] = static_cast<T>(pixel_color.y());
    out[2] = static_cast<T>(pixel_color.z());
}

#endif
