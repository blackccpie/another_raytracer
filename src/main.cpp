#include "tracer_utils.h"

#include "camera.h"
#include "color.h"
#include "engine.h"
#include "gui.h"
#include "tracer_constants.h"
#include "scene_manager.h"

#include <array>
#include <chrono>
#include <iostream>

int main(int argc, char **argv) try
{
    // Image
    constexpr int color_channels = 3;
    scene_alias alias = scene_alias::mesh;
    
    // Optional scene index parameter
    if(argc >= 2)
    {
        alias = static_cast<scene_alias>(std::atoi(argv[1])); // TODO-AM : no error checking! :-(
    } 

    // Scene description
    scene_manager scene_mgr;
    scene world = scene_mgr.build(alias);
    
    // Camera
    vec3 vup(0,1,0);
    auto dist_to_focus = 10.0;

    camera cam(world.lookfrom, world.lookat, vup, world.vfov, tracer_constants::aspect_ratio, world.aperture, dist_to_focus, 0.0, 1.0);
    
    // Render
    std::cout << "output resolution: " << tracer_constants::image_width << "x" << tracer_constants::image_height << std::endl;

    std::array<std::uint8_t,tracer_constants::image_width*tracer_constants::image_height*color_channels> output_image{0};

    engine<tracer_constants::image_width,tracer_constants::image_height,color_channels> eng( cam, engine_mode::adaptive );
    eng.set_scene(world.objects,world.background);
    auto elapsed_ms = eng.run( output_image.data() );
    
    std::cout << std::endl << "Rendering computed in milliseconds: " << elapsed_ms << " ms" << std::endl;
    
    const auto total_rays = tracer_constants::image_width * tracer_constants::image_height * eng.samples_per_pixel;
    const auto ray_processing_rate = static_cast<float>(total_rays) / elapsed_ms;
    
    std::cout << std::endl << "Processing rate: " << ray_processing_rate << "kRay/s" << std::endl;
    
    gui::display( output_image.data(), tracer_constants::image_width, tracer_constants::image_height, 2 );
    
    imageio::save_image("output.png",tracer_constants::image_width,tracer_constants::image_height,color_channels,output_image.data());
    
    return EXIT_SUCCESS;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << std::endl;
}
