#include "rtweekend.h"

#include "aarect.h"
#include "box.h"
#include "bvh.h"
#include "camera.h"
#include "color.h"
#include "gui.h"
#include "hittable_list.h"
#include "material.h"
#include "mesh.h"
#include "sphere.h"
#include "threadpool.h"

#include <array>
#include <chrono>
#include <iostream>

#define PARALLEL_RUN

color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
    hit_record rec;
    
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0,0,0);
    
    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth-1);
}

hittable_list random_scene() {
    hittable_list objects;

    auto ground_checked_material = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(ground_checked_material)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    objects.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    objects.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    objects.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    objects.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    objects.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    hittable_list world;
    world.add(make_shared<bvh_node>(objects, 0, 1));
    
    return world;
}

hittable_list two_spheres() {
    hittable_list objects;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

    objects.add(make_shared<sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

hittable_list two_perlin_spheres() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

hittable_list earth() {
    auto earth_texture = make_shared<image_texture>("textures/earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

    return hittable_list{globe};
}

hittable_list simple_light() {
    hittable_list objects;

    //auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(color(0.4, 0.2, 0.1)/*pertext*/)));
    objects.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(color(0.4, 0.2, 0.1)/*pertext*/)));

    auto difflight = make_shared<diffuse_light>(color(4,4,4));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;
}

hittable_list cornell_box() {
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<box>(point3(130, 0, 65), point3(295, 165, 230), white));
    objects.add(make_shared<box>(point3(265, 0, 295), point3(430, 330, 460), white));

    return objects;
}

hittable_list mesh_scene() {
    mesh m;
    if( m.parse("models/esquisse3.obj") ) {
        return m.build();
    }
    else
        throw std::logic_error("cannot parse input obj file!");
}

int main() try
{
    // Image
    constexpr auto aspect_ratio = 3.0 / 2.0;
    constexpr int image_width = 640;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);
    constexpr int color_channels = 3;
    constexpr int samples_per_pixel = 20;//50;
    constexpr int max_depth = 20;//100;
    constexpr int scene_index = 3;
    
    // World
    hittable_list world;

    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    color background(0,0,0);

    switch (scene_index) {
        case 1:
            world = random_scene();
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            aperture = 0.1;
            break;

        case 2:
            world = two_spheres();
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            break;
            
        case 3:
            world = two_perlin_spheres();
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            break;
            
        case 4:
            world = earth();
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            break;
            
        case 5:
            world = simple_light();
            background = color(0,0,0);
            lookfrom = point3(26,3,6);
            lookat = point3(0,2,0);
            vfov = 20.0;
            break;
        
        case 6:
            world = cornell_box();
            background = color(0,0,0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            break;
            
        case 7:
            world = mesh_scene();
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(1000,800,800);
            //lookfrom = point3(0,1,2);
            lookat = point3(0,0,0);
            vfov = 80.0;
            aperture = 0.1;
            break;
            
        default:
            std::cout << "no scene will be loaded! exiting..." << std::endl;
            return EXIT_FAILURE;
    }
    
    // Camera
    vec3 vup(0,1,0);
    auto dist_to_focus = 10.0;

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);

    // Render
    std::cout << image_width << " " << image_height << std::endl;

    std::array<std::uint8_t,image_width*image_height*color_channels> output_image{0};

    std::atomic<size_t> progress = 0;
    
#ifdef PARALLEL_RUN
    thread_pool tp{4};
    
    auto run_stripe = [&](int j0, int j1) {
        for (int j = j0; j < j1; ++j) {
            int offset = color_channels*j*image_width;
            for (int i = 0; i < image_width; ++i) {
                color pixel_color(0, 0, 0);
                for (int s = 0; s < samples_per_pixel; ++s) {
                    auto u = (i + random_double()) / (image_width-1);
                    auto v = ((image_height-1-j) + random_double()) / (image_height-1); // spatial convention, not image convention!
                    ray r = cam.get_ray(u, v);
                    pixel_color += ray_color(r, background, world, max_depth);
                }
                write_color(output_image.data()+offset, pixel_color, samples_per_pixel);
                offset += color_channels;
            }
            progress++;
        }
    };
    
    using namespace std::chrono_literals;
    const auto start = std::chrono::steady_clock::now();
    
    tp.add_job( [&](){ run_stripe(0,image_height/4); } );
    tp.add_job( [&](){ run_stripe(image_height/4,image_height/2); } );
    tp.add_job( [&](){ run_stripe(image_height/2,3*image_height/4); } );
    tp.add_job( [&](){ run_stripe(3*image_height/4,image_height); } );
    while(true) {
        const auto percent = 100*progress/image_height;
        std::cout << "Computing done @" << 100*progress/image_height << "%\r" << std::flush;
        std::this_thread::sleep_for(100ms);
        if(percent >= 100)
            break;
    }
    tp.wait_all();
#else
    const auto start = std::chrono::steady_clock::now();
    
    for (int j = 0; j < image_height; ++j) {
        progress = j*100/image_height;
        std::cout << "Computing done @" << progress << "%\r" << std::flush;
        int offset = color_channels*j*image_width;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = ((image_height-1-j) + random_double()) / (image_height-1); // spatial convention, not image convention!
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, max_depth);
            }
            write_color(output_image.data()+offset, pixel_color, samples_per_pixel);
            offset += color_channels;
        }
    }
#endif
    
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << std::endl << "Rendering computed in milliseconds: " << elapsed_ms << " ms" << std::endl;
    
    const auto total_rays = image_width * image_height * samples_per_pixel;
    const auto ray_processing_rate = static_cast<float>(total_rays) / elapsed_ms;
    
    std::cout << std::endl << "Processing rate: " << ray_processing_rate << "kRay/s" << std::endl;
    
    gui::display( output_image.data(), image_width, image_height );
    
    return EXIT_SUCCESS;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << std::endl;
}
