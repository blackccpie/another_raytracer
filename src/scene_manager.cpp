#include "scene_manager.h"

#include "aarect.h"
#include "box.h"
#include "bvh.h"
#include "constant_medium.h"
#include "material.h"
#include "mesh.h"
#include "moving_sphere.h"
#include "ressources.h"
#include "sphere.h"

hittable_list scene_manager::_random_scene()
{
    hittable_list objects;

    auto ground_checked_material = std::make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    objects.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(ground_checked_material)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = std::make_shared<lambertian>(albedo);
                    objects.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                    auto center2 = center + vec3(0, random_double(0,.5), 0);
                    objects.add(std::make_shared<moving_sphere>(
                        center, center2, 0.0, 1.0, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = std::make_shared<metal>(albedo, fuzz);
                    objects.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<dielectric>(1.5);
                    objects.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<dielectric>(1.5);
    objects.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
    objects.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    objects.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    hittable_list world;
    world.add(std::make_shared<bvh_node>(objects, 0, 1));
    
    return world;
}

hittable_list scene_manager::_two_spheres()
{
    hittable_list objects;

    auto checker = std::make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

    objects.add(std::make_shared<sphere>(point3(0,-10, 0), 10, std::make_shared<lambertian>(checker)));
    objects.add(std::make_shared<sphere>(point3(0, 10, 0), 10, std::make_shared<lambertian>(checker)));

    return objects;
}

hittable_list scene_manager::_two_perlin_spheres()
{
    hittable_list objects;

    auto pertext = std::make_shared<noise_texture>(4);
    objects.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(pertext)));
    objects.add(std::make_shared<sphere>(point3(0, 2, 0), 2, std::make_shared<lambertian>(pertext)));

    return objects;
}

hittable_list scene_manager::_earth()
{
    auto earth_texture = std::make_shared<image_texture>(ressources::earthmap_texture);
    auto earth_surface = std::make_shared<lambertian>(earth_texture);
    auto globe = std::make_shared<sphere>(point3(0,0,0), 2, earth_surface);

    return hittable_list{globe};
}

hittable_list scene_manager::_simple_light()
{
    hittable_list objects;

    auto pertext = std::make_shared<noise_texture>(4);
    objects.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(pertext)));
    objects.add(std::make_shared<sphere>(point3(0,2,0), 2, std::make_shared<lambertian>(pertext)));

    auto difflight = std::make_shared<diffuse_light>(color(4,4,4));
    objects.add(std::make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;
}

hittable_list scene_manager::_cornell_box()
{
    hittable_list objects;

    auto red   = std::make_shared<lambertian>(color(.65, .05, .05));
    auto white = std::make_shared<lambertian>(color(.73, .73, .73));
    auto green = std::make_shared<lambertian>(color(.12, .45, .15));
    auto light = std::make_shared<diffuse_light>(color(15, 15, 15));

    // TODO-AM : clarify this whole double lighting object thing!!!!!
    world.lights = std::make_shared<xz_rect>(213, 343, 227, 332, 554, std::shared_ptr<material>());
    //world.lights = std::make_shared<flip_face>(std::make_shared<xz_rect>(213, 343, 227, 332, 554, light));

    objects.add(std::make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(std::make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(std::make_shared<flip_face>(std::make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    objects.add(std::make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(std::make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(std::make_shared<xy_rect>(0, 555, 0, 555, 555, white));
    
    std::shared_ptr<hittable> box1 = std::make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = std::make_shared<rotate_y>(box1, 15);
    box1 = std::make_shared<translate>(box1, vec3(265,0,295));
    objects.add(box1);
    
    std::shared_ptr<hittable> box2 = std::make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = std::make_shared<rotate_y>(box2, -18);
    box2 = std::make_shared<translate>(box2, vec3(130,0,65));
    objects.add(box2);

    return objects;
}

hittable_list scene_manager::_cornell_smoke()
{
    hittable_list objects;

    auto red   = std::make_shared<lambertian>(color(.65, .05, .05));
    auto white = std::make_shared<lambertian>(color(.73, .73, .73));
    auto green = std::make_shared<lambertian>(color(.12, .45, .15));
    auto light = std::make_shared<diffuse_light>(color(7, 7, 7));

    objects.add(std::make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(std::make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(std::make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(std::make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(std::make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(std::make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    std::shared_ptr<hittable> box1 = std::make_shared<box>(point3(0,0,0), point3(165,330,165), white);
    box1 = std::make_shared<rotate_y>(box1, 15);
    box1 = std::make_shared<translate>(box1, vec3(265,0,295));

    std::shared_ptr<hittable> box2 = std::make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = std::make_shared<rotate_y>(box2, -18);
    box2 = std::make_shared<translate>(box2, vec3(130,0,65));

    objects.add(std::make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    objects.add(std::make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    return objects;
}

hittable_list scene_manager::_final_scene()
{
    hittable_list boxes1;
    auto ground = std::make_shared<lambertian>(color(0.48, 0.83, 0.53));

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

            boxes1.add(std::make_shared<box>(point3(x0,y0,z0), point3(x1,y1,z1), ground));
        }
    }

    hittable_list objects;

    objects.add(std::make_shared<bvh_node>(boxes1, 0, 1));

    auto light = std::make_shared<diffuse_light>(color(7, 7, 7));
    objects.add(std::make_shared<xz_rect>(123, 423, 147, 412, 554, light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30,0,0);
    auto moving_sphere_material = std::make_shared<lambertian>(color(0.7, 0.3, 0.1));
    objects.add(std::make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

    objects.add(std::make_shared<sphere>(point3(260, 150, 45), 50, std::make_shared<dielectric>(1.5)));
    objects.add(std::make_shared<sphere>(
        point3(0, 150, 145), 50, std::make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
    ));

    auto boundary = std::make_shared<sphere>(point3(360,150,145), 70, std::make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(std::make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
    boundary = std::make_shared<sphere>(point3(0, 0, 0), 5000, std::make_shared<dielectric>(1.5));
    objects.add(std::make_shared<constant_medium>(boundary, .0001, color(1,1,1)));

    auto emat = std::make_shared<lambertian>(std::make_shared<image_texture>(ressources::earthmap_texture));
    objects.add(std::make_shared<sphere>(point3(400,200,400), 100, emat));
    auto pertext = std::make_shared<noise_texture>(0.1);
    objects.add(std::make_shared<sphere>(point3(220,280,300), 80, std::make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = std::make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(std::make_shared<sphere>(point3::random(0,165), 10, white));
    }

    objects.add(std::make_shared<translate>(
        std::make_shared<rotate_y>(
            std::make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
            vec3(-100,270,395)
        )
    );

    return objects;
}

hittable_list scene_manager::_mesh_scene()
{
    mesh m;
    if( m.parse(ressources::capsule_obj_path) ) {
        hittable_list world;
        
        // mesh triangles
        auto triangles = m.build();
        world.add(std::make_shared<bvh_node>(triangles, 0.0, 1.0));
        
        // lighting
        auto light = std::make_shared<diffuse_light>(color(7, 7, 7));
        world.add(std::make_shared<xz_rect>(123, 423, 147, 412, 554, light));
        //world.add(std::make_shared<sphere>(point3(0, 800, 500), 100, light));
        
        // thin mist
        auto boundary = std::make_shared<sphere>(point3(0, 0, 0), 5000, std::make_shared<dielectric>(1.5));
        world.add(std::make_shared<constant_medium>(boundary, .0001, color(1,1,1)));
        return world;
    }
    else
        throw std::logic_error("cannot parse input obj file!");
}

scene scene_manager::build( scene_alias alias )
{
    switch (alias) {
        case scene_alias::random:
            world.objects = _random_scene();
            world.background = color(0.70, 0.80, 1.00);
            world.lookfrom = point3(13,2,3);
            world.lookat = point3(0,0,0);
            world.vfov = 20.0;
            world.aperture = 0.1;
            break;

        case scene_alias::two_spheres:
            world.objects = _two_spheres();
            world.background = color(0.70, 0.80, 1.00);
            world.lookfrom = point3(13,2,3);
            world.lookat = point3(0,0,0);
            world.vfov = 20.0;
            break;
            
        case scene_alias::two_perlin_spheres:
            world.objects = _two_perlin_spheres();
            world.background = color(0.70, 0.80, 1.00);
            world.lookfrom = point3(13,2,3);
            world.lookat = point3(0,0,0);
            world.vfov = 20.0;
            break;
            
        case scene_alias::earth:
            world.objects = _earth();
            world.background = color(0.70, 0.80, 1.00);
            world.lookfrom = point3(13,2,3);
            world.lookat = point3(0,0,0);
            world.vfov = 20.0;
            break;
            
        case scene_alias::simple_light:
            world.objects = _simple_light();
            world.background = color(0,0,0);
            world.lookfrom = point3(26,3,6);
            world.lookat = point3(0,2,0);
            world.vfov = 20.0;
            break;
        
        case scene_alias::cornell_box:
            world.objects = _cornell_box();
            world.background = color(0,0,0);
            world.lookfrom = point3(278, 278, -800);
            world.lookat = point3(278, 278, 0);
            world.vfov = 40.0;
            break;
            
        case scene_alias::cornell_smoke:
            world.objects = _cornell_smoke();
            world.background = color(0,0,0);
            world.lookfrom = point3(278, 278, -800);
            world.lookat = point3(278, 278, 0);
            world.vfov = 40.0;
            break;
            
        case scene_alias::final:
            world.objects = _final_scene();
            world.background = color(0,0,0);
            world.lookfrom = point3(478, 278, -600);
            world.lookat = point3(278, 278, 0);
            world.vfov = 40.0;
            break;
            
        case scene_alias::mesh:
            world.objects = _mesh_scene();
            //world.background = color(0.10, 0.10, 0.10);
            world.background = color(0.70, 0.80, 1.00);
            //house
            //world.lookfrom = point3(-200,300,1100);
            //world.lookat = point3(200,-150,0);
            //dino
            //world.lookfrom = point3(0,15,25);
            //world.lookat = point3(0,10,0);
            //cow
            //world.lookfrom = point3(4,2,6);
            //world.lookat = point3(2,0,0);
            //capsule
            world.lookfrom = point3(2,2,1);
            world.lookat = point3(0,0,0);
            world.vfov = 75.0;
            //world.aperture = 0.1;
            break;
            
        default:
            throw std::logic_error("unkwnown scene requested!");
    }

    return world;
}
    
   
