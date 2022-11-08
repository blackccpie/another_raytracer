#include "rtweekend.h"

#include "camera.h"
#include "color.h"
#include "gui.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

#include <array>
#include <chrono>
#include <iostream>

color ray_color(const ray& r, const hittable& world, int depth) {
    hit_record rec;
    
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0,0,0);
    
    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth-1);
        return color(0,0,0);
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t*color(0.5, 0.7, 1.0);
}


int main()
{
    // Image
    constexpr auto aspect_ratio = 16.0 / 9.0;
    constexpr int image_width = 640;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);
    constexpr int color_channels = 3;
    constexpr int samples_per_pixel = 20;
    constexpr int max_depth = 50;

    // World
    hittable_list world;
    
    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(color(0.7, 0.3, 0.3));
    auto material_left   = make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);
    auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);
    
    world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.0),   0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));
    
    // Camera
    camera cam;

    // Render
    std::cout << image_width << " " << image_height << std::endl;

    std::array<std::uint8_t,image_width*image_height*color_channels> output_image{0};

    auto start = std::chrono::steady_clock::now();
    
    for (int j = 0; j < image_height; ++j) {
        int offset = color_channels*j*image_width;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = ((image_height-1-j) + random_double()) / (image_height-1); // spatial convention, not image convention!
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            write_color(output_image.data()+offset, pixel_color, samples_per_pixel);
            offset += color_channels;
        }
    }
    
    auto end = std::chrono::steady_clock::now();
    
    std::cout << "Rendering computed in milliseconds: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    
    gui::display( output_image.data(), image_width, image_height );
}
