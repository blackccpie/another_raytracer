cmake_minimum_required (VERSION 3.24)
project (another_raytracer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(RAYCASTER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
configure_file(src/ressources.h.in ressources.h @ONLY)

set (sources_list
    src/engine/hittable.cpp
    src/engine/hittable_list.cpp
    src/primitives/aarect.cpp
    src/primitives/box.cpp
    src/primitives/bvh.cpp
    src/utils/gui.cpp
    src/utils/imageio.cpp
    src/main.cpp
    src/scene_manager.cpp
)

set (headers_list
    src/scene_manager.h
    src/core/color.h
    src/core/onb.h
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
    src/utils/threadpool.h
    src/utils/tracer_utils.h
)

add_executable(another_raytracer ${sources_list} ${headers_list})

if(NOT WIN32)
    # The argument size is a hexadecimal number with
    # an optional leading 0x. The size should be an even multiple of 4KB, that is the
    # last three hexadecimal digits should be zero
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-stack_size -Wl,0x1000000") # 16Mo stack limit

    target_compile_options(another_raytracer PRIVATE "-Wall" "-Wconversion")

endif()

target_include_directories(another_raytracer SYSTEM PRIVATE
    3rd_parties/CImg
    3rd_parties/rapidobj
    3rd_parties/stb
)

target_include_directories(another_raytracer PRIVATE
    src/core
    src/engine
    src/primitives
    src/rendering
    src/utils
)

if(NOT WIN32)
    target_include_directories(another_raytracer SYSTEM PRIVATE
        /opt/X11/include
    )

    target_link_directories(another_raytracer PRIVATE
        /opt/X11/lib
    )

    target_link_libraries(another_raytracer PRIVATE
        X11
        pthread
    )
endif()