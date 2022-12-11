#include "rtweekend.h"

#include "ressources.h"

#include "aarect.h"
#include "box.h"
#include "bvh.h"
#include "camera.h"
#include "color.h"
#include "constant_medium.h"
#include "engine.h"
#include "gui.h"
#include "hittable_list.h"
#include "material.h"
#include "mesh.h"
#include "moving_sphere.h"
#include "sphere.h"

#include <array>
#include <chrono>
#include <iostream>

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
                    auto center2 = center + vec3(0, random_double(0,.5), 0);
                    objects.add(make_shared<moving_sphere>(
                        center, center2, 0.0, 1.0, 0.2, sphere_material));
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

    auto pertext = make_shared<noise_texture>();
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

hittable_list earth() {
    auto earth_texture = make_shared<image_texture>(ressources::earthmap_texture);
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

    return hittable_list{globe};
}

hittable_list simple_light() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

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
    
    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    objects.add(box1);
    
    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));
    objects.add(box2);

    return objects;
}

hittable_list cornell_smoke() {
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));

    objects.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    objects.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    return objects;
}

hittable_list final_scene() {
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1,101);
            auto z1 = z0 + w;

            boxes1.add(make_shared<box>(point3(x0,y0,z0), point3(x1,y1,z1), ground));
        }
    }

    hittable_list objects;

    objects.add(make_shared<bvh_node>(boxes1, 0, 1));

    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30,0,0);
    auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

    objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    objects.add(make_shared<sphere>(
        point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
    ));

    auto boundary = make_shared<sphere>(point3(360,150,145), 70, make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
    boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
    objects.add(make_shared<constant_medium>(boundary, .0001, color(1,1,1)));

    auto emat = make_shared<lambertian>(make_shared<image_texture>(ressources::earthmap_texture));
    objects.add(make_shared<sphere>(point3(400,200,400), 100, emat));
    auto pertext = make_shared<noise_texture>(0.1);
    objects.add(make_shared<sphere>(point3(220,280,300), 80, make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<sphere>(point3::random(0,165), 10, white));
    }

    objects.add(make_shared<translate>(
        make_shared<rotate_y>(
            make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
            vec3(-100,270,395)
        )
    );

    return objects;
}

hittable_list mesh_scene() {
    mesh m;
    if( m.parse(ressources::capsule_obj_path) ) {
        hittable_list world;
        
        // mesh triangles
        auto triangles = m.build();
        world.add(make_shared<bvh_node>(triangles, 0.0, 1.0));
        
        // lighting
        auto light = make_shared<diffuse_light>(color(7, 7, 7));
        world.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));
        //world.add(make_shared<sphere>(point3(0, 800, 500), 100, light));
        
        // thin mist
        auto boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
        world.add(make_shared<constant_medium>(boundary, .0001, color(1,1,1)));
        return world;
    }
    else
        throw std::logic_error("cannot parse input obj file!");
}

int main(int argc, char **argv) try
{
    // Image
    constexpr auto aspect_ratio = 16.0 / 9.0;
    constexpr int image_width = 640;
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
            world = cornell_smoke();
            background = color(0,0,0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            break;
            
        case 8:
            world = final_scene();
            background = color(0,0,0);
            lookfrom = point3(478, 278, -600);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            break;
            
        case 9:
            world = mesh_scene();
            background = color(0.10, 0.10, 0.10);
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
            lookfrom = point3(2,2,0);
            lookat = point3(0,0,0);
            vfov = 75.0;
            aperture = 0.1;
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

    engine<image_width,image_height,color_channels> eng( cam, engine_mode::parallel_images );
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
