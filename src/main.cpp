#include "tracer_utils.h"

#include "camera.h"
#include "color.h"
#include "engine.h"
#include "frame_allocator.h"
#include "gui.h"
#include "tracer_constants.h"
#include "scene_manager.h"

#include <array>
#include <chrono>
#include <iostream>

namespace tc = tracer_constants;

int main(int argc, char **argv) try
{
    // Default scene
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
    camera cam(world.lookfrom, world.lookat, vup, world.vfov, tc::aspect_ratio, world.aperture, dist_to_focus, 0.0, 1.0);
    
    // Render
    std::cout << "output resolution: " << tc::image_width << "x" << tc::image_height << std::endl;

    // Allocate rendering frame
    frame_allocator<std::uint8_t,tc::frame_size,1> frame_alloc;
    auto output_image = frame_alloc.get_frame(0);
    output_image.fill(0);

    engine<tc::image_width,tc::image_height,tc::color_channels> eng( cam, engine_mode::adaptive );
    eng.set_scene(world.objects,world.background);
    auto elapsed_ms = eng.run( output_image.data() );
    
    std::cout << std::endl << "Rendering computed in milliseconds: " << elapsed_ms << " ms" << std::endl;
    
    const auto total_rays = tc::image_width * tc::image_height * tracer_constants::samples_per_pixel;
    const auto ray_processing_rate = static_cast<float>(total_rays) / elapsed_ms;
    
    std::cout << std::endl << "Processing rate: " << ray_processing_rate << "kRay/s" << std::endl;
    
    gui::display( output_image.data(), tc::image_width, tc::image_height, 2 );
    
    imageio::save_image("output.png",tc::image_width,tc::image_height,tc::color_channels,output_image.data());
    
    return EXIT_SUCCESS;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << std::endl;
}
