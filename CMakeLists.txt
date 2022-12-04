cmake_minimum_required (VERSION 3.24)
project (raycaster)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (sources_list
    src/utils/gui.cpp
    src/main.cpp
)

set (headers_list
    src/core/color.h
    src/core/ray.h
    src/core/vec3.h
    src/engine/camera.h
    src/engine/constant_medium.h
    src/engine/engine.h
    src/engine/hittable.h
    src/engine/hittable_list.h
    src/primitives/aabb.h
    src/primitives/aarect.h
    src/primitives/box.h
    src/primitives/bvh.h
    src/primitives/mesh.h
    src/primitives/moving_sphere.h
    src/primitives/sphere.h
    src/primitives/triangle.h
    src/rendering/material.h
    src/rendering/perlin.h
    src/rendering/texture.h
    src/utils/gui.h
    src/utils/imageio.h
    src/utils/rtweekend.h
    src/utils/threadpool.h
)

add_executable(raycaster ${sources_list} ${headers_list})

target_include_directories(raycaster PRIVATE
    /opt/X11/include
    3rd_parties/CImg
    3rd_parties/rapidobj
    3rd_parties/stb
    src/core
    src/engine
    src/primitives
    src/rendering
    src/utils
)

target_link_directories(raycaster PRIVATE
    /opt/X11/lib
)

target_link_libraries(raycaster PRIVATE
    X11
)