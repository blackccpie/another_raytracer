#ifndef ENGINE_H
#define ENGINE_H

#include "threadpool.h"

#define PARALLEL_RUN

class engine
{
public:
    engine( const camera& _cam) : cam(_cam) {}
    
    void set_scene(hittable_list _world, color _background)
    {
        world = _world;
        background = _background;
    }
    
    int run( std::uint8_t* output_image, int image_width, int image_height, int color_channels)
    {
        if( world.empty() )
        {
            std::cerr << "Invalid input scene!" << std::endl;
            return -1;
        }
        
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
                        pixel_color += _ray_color(r, background, world, max_depth);
                    }
                    write_color(output_image+offset, pixel_color, samples_per_pixel);
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
                    pixel_color += _ray_color(r, background, world, max_depth);
                }
                write_color(output_image+offset, pixel_color, samples_per_pixel);
                offset += color_channels;
            }
        }
#endif
        
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return static_cast<int>(elapsed_ms);
    }
    
private:
    
    color _ray_color(const ray& r, const color& background, const hittable& world, int depth) {
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
        
        return emitted + attenuation * _ray_color(scattered, background, world, depth-1);
    }
    
private:
    const camera& cam;
    hittable_list world;
    color background{0,0,0};

public:
    static constexpr int samples_per_pixel = 200;
    static constexpr int max_depth = 20;
};

#endif
