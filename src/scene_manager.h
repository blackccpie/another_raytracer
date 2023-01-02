#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "hittable_list.h"

class scene_manager
{
public:
    hittable_list build( int scene_index );
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