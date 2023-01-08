#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "hittable_list.h"

struct scene
{
    point3 lookfrom;
    point3 lookat;
    double vfov = 40.;
    double aperture = 0.;
    color background{0,0,0};
    hittable_list objects;
};

enum class scene_alias
{
    random = 1,
    two_spheres = 2,
    two_perlin_spheres = 3,
    earth = 4,
    simple_light = 5,
    cornell_box = 6,
    cornell_smoke = 7,
    final = 8,
    mesh = 9
};

class scene_manager
{
public:
    scene build( scene_alias alias );
private:
    hittable_list _random_scene();
    hittable_list _two_spheres();
    hittable_list _two_perlin_spheres();
    hittable_list _earth();
    hittable_list _simple_light();
    hittable_list _cornell_box();
    hittable_list _cornell_smoke();
    hittable_list _final_scene();
    hittable_list _mesh_scene();
};

#endif