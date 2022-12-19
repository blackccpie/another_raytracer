#ifndef ENGINE_H
#define ENGINE_H

#include "threadpool.h"

enum class engine_mode
    {
        single,
        adaptive,
        parallel_stripes,
        parallel_images
    };

template<int image_width, int image_height, int color_channels>
class engine
{
public:
    engine( const camera& _cam, engine_mode _m) : m(_m), cam(_cam) {}
    
    void set_scene(hittable_list _world, color _background)
    {
        world = _world;
        background = _background;
    }
    
    int run( std::uint8_t* output_image)
    {
        if( world.empty() )
        {
            std::cerr << "Invalid input scene!" << std::endl;
            return -1;
        }
        
        switch(m)
        {
        case engine_mode::single:
        default:
            return _run_single(output_image);
        case engine_mode::adaptive:
            return _run_adaptive(output_image);
        case engine_mode::parallel_stripes:
            return _run_parallel_stripes(output_image);
        case engine_mode::parallel_images:
            return _run_parallel_images(output_image);
        }
    }
    
private:
    
    inline color _stochastic_sample(int i, int j, int _samples_per_pixel = samples_per_pixel)
    {
        color pixel_color(0, 0, 0);
        for (int s = 0; s < _samples_per_pixel; ++s) {
            auto u = (i + random_double()) / (image_width-1);
            auto v = ((image_height-1-j) + random_double()) / (image_height-1); // spatial convention, not image convention!
            ray r = cam.get_ray(u, v);
            pixel_color += _ray_color(r, background, world, max_depth);
        }
        return pixel_color;
    }

    int _run_single(std::uint8_t* output_image)
    {
        int progress = 0;

        const auto start = std::chrono::steady_clock::now();
        
        for (int j = 0; j < image_height; ++j) {
            progress = j*100/image_height;
            std::cout << "Computing done @" << progress << "%\r" << std::flush;
            int offset = color_channels*j*image_width;
            for (int i = 0; i < image_width; ++i) {
                write_color(output_image+offset, _stochastic_sample(i,j), samples_per_pixel);
                offset += color_channels;
            }
        }

        const auto end = std::chrono::steady_clock::now();
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return static_cast<int>(elapsed_ms);
    }

    bool _compute_corners_heuristic(int* upleft_corner, size_t square_length, size_t square_line_offset)
    {
        constexpr int subdivide_thresh = 100;

        const auto [cr1,cg1,cb1] = std::tuple{  upleft_corner[0], 
                                                upleft_corner[1], 
                                                upleft_corner[2] };
        const auto [cr2,cg2,cb2] = std::tuple{  upleft_corner[0+3*square_length], 
                                                upleft_corner[1+3*square_length], 
                                                upleft_corner[2+3*square_length] };
        if(cr2 == -128) return false;
        const auto [cr3,cg3,cb3] = std::tuple{  upleft_corner[0+3*square_length*square_line_offset], 
                                                upleft_corner[1+3*square_length*square_line_offset], 
                                                upleft_corner[2+3*square_length*square_line_offset] };
        if(cr3 == -128) return false;
        const auto [cr4,cg4,cb4] = std::tuple{  upleft_corner[0+3*square_length*square_line_offset+3*square_length], 
                                                upleft_corner[1+3*square_length*square_line_offset+3*square_length], 
                                                upleft_corner[2+3*square_length*square_line_offset+3*square_length] };
        if(cr4 == -128) return false;

        auto flag_subnodes = [&]() {
            upleft_corner[0+3*square_length/2] = -2;
            upleft_corner[0+3*(square_length/2)*square_line_offset] = -2;
            upleft_corner[0+3*(square_length/2)*square_line_offset+3*square_length] = -2;
            upleft_corner[0+3*square_length*square_line_offset+3*(square_length/2)] = -2;
            upleft_corner[0+3*(square_length/2)*square_line_offset+3*(square_length/2)] = -2;
            return true;
        };

        const auto distance1 = (cr1 - cr2)*(cr1 - cr2) + (cg1 - cg2)*(cg1 - cg2) + (cb1 - cb2)*(cb1 - cb2);
        if( distance1 > subdivide_thresh) return flag_subnodes();
        const auto distance2 = (cr2 - cr4)*(cr2 - cr4) + (cg2 - cg4)*(cg2 - cg4) + (cb2 - cb4)*(cb2 - cb4);
        if( distance2 > subdivide_thresh) return flag_subnodes();
        const auto distance3 = (cr4 - cr3)*(cr4 - cr3) + (cg4 - cg3)*(cg4 - cg3) + (cb4 - cb3)*(cb4 - cb3);
        if( distance3 > subdivide_thresh) return flag_subnodes();
        const auto distance4 = (cr3 - cr1)*(cr3 - cr1) + (cg3 - cg1)*(cg3 - cg1) + (cb3 - cb1)*(cb3 - cb1);
        if( distance4 > subdivide_thresh) return flag_subnodes();

        return false;
    }

    int _run_adaptive(std::uint8_t* output_image)
    {
        int progress = 0;

        std::array<int,image_width*image_height*color_channels> work_image;
        work_image.fill(-128);
        
        const auto start = std::chrono::steady_clock::now();
        
        for (int j = 0; j < image_height; j+=8) {
            progress = j*100/image_height;
            std::cout << "Computing done @" << progress << "%\r" << std::flush;
            int offset = color_channels*j*image_width;
            for (int i = 0; i < image_width; i+=8) {
                write_color<int>(work_image.data()+offset, _stochastic_sample(i,j), samples_per_pixel);
                offset += 8*color_channels;
            }
        }

        std::cout << "STEP1 : 8px finished" << std::endl;

        progress = 0;

        for (int j = 0; j < image_height-8; j+=4) {
            progress = j*100/image_height;
            std::cout << "Computing done @" << progress << "%\r" << std::flush;
            std::ptrdiff_t offset = color_channels*j*image_width;
            for (int i = 0; i < image_width-8; i+=4) {
                const auto upleft_corner = work_image.data()+offset;
                if(upleft_corner[0] >= 0) // computed 8px node
                {
                    _compute_corners_heuristic(upleft_corner,8,image_width);
                }
                if(upleft_corner[0] == -2) // 4px node flagged
                {
                    write_color<int>(work_image.data()+offset, _stochastic_sample(i,j), samples_per_pixel);
                }
                offset += 4*color_channels;
            }
        }

        std::cout << "STEP2 : 4px finished" << std::endl;

        progress = 0;

        for (int j = 0; j < image_height-8; j+=2) {
            progress = j*100/image_height;
            std::cout << "Computing done @" << progress << "%\r" << std::flush;
            std::ptrdiff_t offset = color_channels*j*image_width;
            for (int i = 0; i < image_width-8; i+=2) {
                const auto upleft_corner = work_image.data()+offset;
                if(upleft_corner[0] >= 0) // computed 4px node
                {
                    _compute_corners_heuristic(upleft_corner,4,image_width);
                }
                if(upleft_corner[0] == -2) // 2px node flagged
                {
                    write_color<int>(work_image.data()+offset, _stochastic_sample(i,j), samples_per_pixel);
                }
                offset += 2*color_channels;
            }
        }

        std::cout << "STEP3 : 2px finished" << std::endl;

        progress = 0;

        for (int j = 0; j < image_height-8; ++j) {
            progress = j*100/image_height;
            std::cout << "Computing done @" << progress << "%\r" << std::flush;
            std::ptrdiff_t offset = color_channels*j*image_width;
            for (int i = 0; i < image_width-8; ++i) {
                const auto upleft_corner = work_image.data()+offset;
                if(upleft_corner[0] >= 0) // computed 2px node
                {
                    _compute_corners_heuristic(upleft_corner,2,image_width);
                }
                if(upleft_corner[0] == -2) // 1px node flagged
                {
                    write_color<int>(work_image.data()+offset, _stochastic_sample(i,j), samples_per_pixel);
                }
                offset += color_channels;
            }
        }

        std::cout << "STEP4 : 1px finished" << std::endl;

        std::transform(work_image.cbegin(), work_image.cend(), output_image, 
            [](const int& val){ return static_cast<std::uint8_t>(val); });

        const auto end = std::chrono::steady_clock::now();
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return static_cast<int>(elapsed_ms);
    }

    int _run_parallel_stripes(std::uint8_t* output_image)
    {
        std::atomic<int> progress = 0;

        thread_pool tp{4};
        
        auto run_stripe = [&](int j0, int j1) {
            for (int j = j0; j < j1; ++j) {
                int offset = color_channels*j*image_width;
                for (int i = 0; i < image_width; ++i) {
                    write_color(output_image+offset, _stochastic_sample(i,j), samples_per_pixel);
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
            std::cout << "Computing done @" << percent << "%\r" << std::flush;
            std::this_thread::sleep_for(100ms);
            if(percent >= 100)
                break;
        }
        tp.wait_all();

        const auto end = std::chrono::steady_clock::now();
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return static_cast<int>(elapsed_ms);
    }

    int _run_parallel_images(std::uint8_t* output_image)
    {
        std::atomic<int> progress = 0;

        thread_pool tp{4};

        std::array<float,image_width*image_height*color_channels> work_image1{0};
        std::array<float,image_width*image_height*color_channels> work_image2{0};
        std::array<float,image_width*image_height*color_channels> work_image3{0};
        std::array<float,image_width*image_height*color_channels> work_image4{0};

        auto run_image = [&](float* partial_image,int small_samples_per_pixel) {
            for (int j = 0; j < image_height; ++j) {
                int offset = color_channels*j*image_width;
                for (int i = 0; i < image_width; ++i) {
                    const color pixel_color = _stochastic_sample(i,j,small_samples_per_pixel);
                    auto* out = partial_image+offset;
                    out[0] = static_cast<float>(pixel_color[0]);
                    out[1] = static_cast<float>(pixel_color[1]);
                    out[2] = static_cast<float>(pixel_color[2]);
                    offset += color_channels;
                }
                progress++;
            }
        };

        using namespace std::chrono_literals;
        const auto start = std::chrono::steady_clock::now();

        tp.add_job( [&](){ run_image(work_image1.data(), samples_per_pixel/4); } );
        tp.add_job( [&](){ run_image(work_image2.data(), samples_per_pixel/4); } );
        tp.add_job( [&](){ run_image(work_image3.data(), samples_per_pixel/4); } );
        tp.add_job( [&](){ run_image(work_image4.data(), samples_per_pixel/4); } );
        while(true) {
            const auto percent = 100*progress/(4*image_height);
            std::cout << "Computing done @" << percent << "%\r" << std::flush;
            std::this_thread::sleep_for(100ms);
            if(percent >= 100)
                break;
        }
        tp.wait_all();

        for (int j = 0; j < image_height; ++j) {
            int offset = color_channels*j*image_width;
            for (int i = 0; i < image_width; ++i) {
                auto* wk1 = work_image1.data()+offset;
                auto* wk2 = work_image2.data()+offset;
                auto* wk3 = work_image3.data()+offset;
                auto* wk4 = work_image4.data()+offset;
                color pixel_color1(wk1[0], wk1[1], wk1[2]);
                color pixel_color2(wk2[0], wk2[1], wk2[2]);
                color pixel_color3(wk3[0], wk3[1], wk3[2]);
                color pixel_color4(wk4[0], wk4[1], wk4[2]);
                color pixel_acc = pixel_color1+pixel_color2+pixel_color3+pixel_color4;
                auto* out = output_image+offset;
                write_color(out, pixel_acc, samples_per_pixel);
                offset += color_channels;
            }
        }

        const auto end = std::chrono::steady_clock::now();
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return static_cast<int>(elapsed_ms);
    }

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
    engine_mode m = engine_mode::single;
    const camera& cam;
    hittable_list world;
    color background{0,0,0};

public:
    static constexpr int samples_per_pixel = 100;
    static constexpr int max_depth = 50;
};

#endif
