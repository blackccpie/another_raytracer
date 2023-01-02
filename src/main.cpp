#include "rtweekend.h"

#include "camera.h"
#include "color.h"
#include "engine.h"
#include "gui.h"
#include "scene_manager.h"

#include <array>
#include <chrono>
#include <iostream>

int main(int argc, char **argv) try
{
    // Image
    constexpr auto aspect_ratio = 4.0 / 3.0;
    constexpr int image_width = 720;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);
    constexpr int color_channels = 3;
    int scene_index = 9;
    
    // Optional scene index parameter
    if(argc >= 2)
    {
        scene_index = std::atoi(argv[1]);
    } 

    // World
    hittable_list world;

    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    color background(0,0,0);

    scene_manager scene;

    switch (scene_index) {
        case 1:
            world = scene.build(1);
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            aperture = 0.1;
            break;

        case 2:
            world = scene.build(2);
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            break;
            
        case 3:
            world = scene.build(3);
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            break;
            
        case 4:
            world = scene.build(4);
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            break;
            
        case 5:
            world = scene.build(5);
            background = color(0,0,0);
            lookfrom = point3(26,3,6);
            lookat = point3(0,2,0);
            vfov = 20.0;
            break;
        
        case 6:
            world = scene.build(6);
            background = color(0,0,0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            break;
            
        case 7:
            world = scene.build(7);
            background = color(0,0,0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            break;
            
        case 8:
            world = scene.build(8);
            background = color(0,0,0);
            lookfrom = point3(478, 278, -600);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            break;
            
        case 9:
            world = scene.build(9);
            //background = color(0.10, 0.10, 0.10);
            background = color(0.70, 0.80, 1.00);
            //house
            //lookfrom = point3(-200,300,1100);
            //lookat = point3(200,-150,0);
            //dino
            //lookfrom = point3(0,15,25);
            //lookat = point3(0,10,0);
            //cow
            //lookfrom = point3(4,2,6);
            //lookat = point3(2,0,0);
            //capsule
            lookfrom = point3(2,2,1);
            lookat = point3(0,0,0);
            vfov = 75.0;
            //aperture = 0.1;
            break;
            
        default:
            std::cout << "no scene will be loaded! exiting..." << std::endl;
            return EXIT_FAILURE;
    }
    
    // Camera
    vec3 vup(0,1,0);
    auto dist_to_focus = 10.0;

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
    
    // Render
    std::cout << image_width << " " << image_height << std::endl;

    std::array<std::uint8_t,image_width*image_height*color_channels> output_image{0};

    engine<image_width,image_height,color_channels> eng( cam, engine_mode::adaptive );
    eng.set_scene(world,background);
    auto elapsed_ms = eng.run( output_image.data() );
    
    std::cout << std::endl << "Rendering computed in milliseconds: " << elapsed_ms << " ms" << std::endl;
    
    const auto total_rays = image_width * image_height * eng.samples_per_pixel;
    const auto ray_processing_rate = static_cast<float>(total_rays) / elapsed_ms;
    
    std::cout << std::endl << "Processing rate: " << ray_processing_rate << "kRay/s" << std::endl;
    
    gui::display( output_image.data(), image_width, image_height );
    
    imageio::save_image("output.png",image_width,image_height,color_channels,output_image.data());
    
    return EXIT_SUCCESS;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << std::endl;
}
