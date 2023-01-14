#ifndef ENGINE_H
#define ENGINE_H

#include "frame_allocator.h"
#include "gui.h"
#include "material.h"
#include "hittable_list.h"
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
        
        std::cout << "--> engine raycasting start" << std::endl;

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

        std::cout << "--> engine raycasting stop" << std::endl;
    }
    
private:

    inline color _stochastic_sample(int i, int j, int _samples_per_pixel = tracer_constants::samples_per_pixel)
    {
        color pixel_color(0, 0, 0);
        for (int s = 0; s < _samples_per_pixel; ++s) {
            auto u = (i + random_double()) / (image_width-1);
            auto v = ((image_height-1-j) + random_double()) / (image_height-1); // spatial convention, not image convention!
            ray r = cam.get_ray(u, v);
            pixel_color += _ray_color(r, background, world, tracer_constants::max_depth);
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
                write_color(output_image+offset, _stochastic_sample(i,j), tracer_constants::samples_per_pixel);
                offset += color_channels;
            }
        }

        const auto end = std::chrono::steady_clock::now();
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return static_cast<int>(elapsed_ms);
    }

    bool _compute_corners_heuristic(int* upleft_corner, size_t square_length, size_t square_line_offset/*, int p, int q*/)
    {
        constexpr int subdivide_thresh = 100;

        auto rgb_tuple_accessor = [&]<typename T>(T* data,size_t i, size_t j) { 
            
            T* pix_start = data+i*color_channels+j*color_channels*image_width;
            return std::tuple{  pix_start[0], 
                                pix_start[1], 
                                pix_start[2] };
        };

        const auto [cr1,cg1,cb1] = rgb_tuple_accessor(upleft_corner,0,0);
        const auto [cr2,cg2,cb2] = rgb_tuple_accessor(upleft_corner,square_length-1,0);
        const auto [cr3,cg3,cb3] = rgb_tuple_accessor(upleft_corner,0,square_length-1);
        const auto [cr4,cg4,cb4] = rgb_tuple_accessor(upleft_corner,square_length-1,square_length-1);

        const auto distance1 = (cr1 - cr2)*(cr1 - cr2) + (cg1 - cg2)*(cg1 - cg2) + (cb1 - cb2)*(cb1 - cb2);
        if( distance1 > subdivide_thresh) return true;
        const auto distance2 = (cr2 - cr4)*(cr2 - cr4) + (cg2 - cg4)*(cg2 - cg4) + (cb2 - cb4)*(cb2 - cb4);
        if( distance2 > subdivide_thresh) return true;
        const auto distance3 = (cr4 - cr3)*(cr4 - cr3) + (cg4 - cg3)*(cg4 - cg3) + (cb4 - cb3)*(cb4 - cb3);
        if( distance3 > subdivide_thresh) return true;
        const auto distance4 = (cr3 - cr1)*(cr3 - cr1) + (cg3 - cg1)*(cg3 - cg1) + (cb3 - cb1)*(cb3 - cb1);
        if( distance4 > subdivide_thresh) return true;

        /*write_color<int>(   upleft_corner+(square_length/2)*color_channels+(square_length/2)*color_channels*image_width, 
                            _stochastic_sample(p+static_cast<int>(square_length/2),q+static_cast<int>(square_length/2)),
                            tracer_constants::samples_per_pixel);

        const auto [crx,cgx,cbx] = rgb_tuple_accessor(upleft_corner,square_length/2,square_length/2);
        const auto distancex1 = (cr1 - crx)*(cr1 - crx) + (cg1 - cgx)*(cg1 - cgx) + (cb1 - cbx)*(cb1 - cbx);
        if( distancex1 > subdivide_thresh) return true;
        const auto distancex2 = (cr2 - crx)*(cr2 - crx) + (cg2 - cgx)*(cg2 - cgx) + (cb2 - cbx)*(cb2 - cbx);
        if( distancex2 > subdivide_thresh) return true;
        const auto distancex3 = (cr3 - cr1)*(cr3 - crx) + (cg3 - cgx)*(cg3 - cgx) + (cb3 - cbx)*(cb3 - cbx);
        if( distancex3 > subdivide_thresh) return true;
        const auto distancex4 = (cr4 - cr3)*(cr4 - crx) + (cg4 - cgx)*(cg4 - cgx) + (cb4 - cbx)*(cb4 - cbx);
        if( distancex4 > subdivide_thresh) return true;*/

        return false;
    }

    color _interpolate(int x, int y, int x1, int x2, int y1, int y2, color Q11, color Q12, color Q21, color Q22)
    {
        // https://www.omnicalculator.com/math/bilinear-interpolation

        const auto xdiff = x2-x1;
        const color R1 = (x2 - x)*Q11/xdiff +  (x-x1)*Q21/xdiff;
        const color R2 = (x2 - x)*Q12/xdiff +  (x-x1)*Q22/xdiff; // TODO-AM : could be precomputed

        const auto ydiff = y2-y1;
        return (y2-y)*R1/ydiff + (y-y1)*R2/ydiff;
    }

    int _run_adaptive(std::uint8_t* output_image)
    {
        std::atomic<int> progress = 0;

        thread_pool tp{4};

        dynamic_gui dgui(image_width, image_height, 2, "Adaptive");

        const auto rgb_accessor = [&]<typename T>(T* data,int i, int j) -> T* { 
            return data+i*color_channels+j*color_channels*image_width;
        };

        const auto to_color_func = [&]<typename T>(T* data, int i, int j) { 
            std::ptrdiff_t offset = i*color_channels+j*color_channels*image_width;
            return color{ 
                static_cast<double>(data[0+offset]),
                static_cast<double>(data[1+offset]),
                static_cast<double>(data[2+offset])
            };
        };

        frame_allocator<int,tracer_constants::frame_size,1> frame_alloc; // TODO-AM : int image really needed?
        auto& work_image = frame_alloc.get_frame(0,-1);

        using namespace std::chrono_literals;
        const auto start = std::chrono::steady_clock::now();

        constexpr int big_square_size = 12;
        constexpr int mid_square_size = big_square_size/2;
        constexpr int small_square_size = mid_square_size/2;
        static_assert(big_square_size % 3 == 0 && big_square_size % 2 == 0); // must be even and a multiple of 3!!
        if(image_width % big_square_size != 0 || image_height % big_square_size != 0) 
            throw std::logic_error( "for adaptive strategy image size should perfectly fit big square size for now!!");

        /* interpolate square content */
        const auto interpolate_square = [&](int* data, int i, int j, int square_size)
        {
            const auto pixel_upleft = rgb_accessor(data,i,j);
            const auto x1 = i;
            const auto x2 = i+square_size-1;
            const auto y1 = j;
            const auto y2 = j+square_size-1;
            const auto color1 = to_color_func(data,x1,y1);
            const auto color2 = to_color_func(data,x1,y2);
            const auto color3 = to_color_func(data,x2,y1);
            const auto color4 = to_color_func(data,x2,y2);

            for(int l=0; l<square_size; ++l)
            {
                for(int k=0; k<square_size; ++k)
                {
                    const auto pixel_interp = rgb_accessor(pixel_upleft,k,l);
                    if(pixel_interp[0] >= 0) // don't overwrite updated pixel
                        continue;

                    // compute interpolated colors
                    const color interp_color = _interpolate(
                        i+k, j+l,
                        x1, x2, y1, y2,
                        color1,
                        color2,
                        color3,
                        color4
                    );
                    // write pixel
                    write_color_raw(pixel_interp, interp_color); // NOTE: don't apply gamma correction here!
                }
            }
        };

        /* evaluate square corner colors */
        const auto evaluate_corners = [&](int* data, int i, int j, int square_size)
        {
            const auto pixel_upleft = rgb_accessor(data,i,j);
            const auto pixel_upright = rgb_accessor(data,i+square_size-1,j);
            const auto pixel_bottomleft = rgb_accessor(data,i,j+square_size-1);
            const auto pixel_bottomright = rgb_accessor(data,i+square_size-1,j+square_size-1);
            write_color<int>(pixel_upleft, _stochastic_sample(i,j), tracer_constants::samples_per_pixel);
            write_color<int>(pixel_upright, _stochastic_sample(i+square_size-1,j), tracer_constants::samples_per_pixel);
            write_color<int>(pixel_bottomleft, _stochastic_sample(i,j+square_size-1), tracer_constants::samples_per_pixel);
            write_color<int>(pixel_bottomright, _stochastic_sample(i+square_size-1,j+square_size-1), tracer_constants::samples_per_pixel);
        };

        /* whole process on the "big square" */
        const auto process_square = [&](int i ,int j)
        {
            const auto pixel_upleft = rgb_accessor(work_image.data(),i,j);
            evaluate_corners(work_image.data(),i,j,big_square_size);

            // do we need smaller resolution
            bool need_subsampling = _compute_corners_heuristic(pixel_upleft,big_square_size,image_width/*,i,j*/);

            if(need_subsampling) 
            {
                for(int l=j; l<j+big_square_size; l+= mid_square_size) {
                    for(int k=i; k<i+big_square_size; k+=mid_square_size) {

                        const auto pixel_upleft2 = rgb_accessor(work_image.data(),k,l);
                        evaluate_corners(work_image.data(),k,l,mid_square_size);

                        // do we need smaller resolution
                        bool need_subsampling2 = _compute_corners_heuristic(pixel_upleft2,mid_square_size,image_width/*,k,l*/);

                        if(need_subsampling2)
                        {
                            for(int n=l; n<l+mid_square_size; n+=small_square_size) {
                                for(int m=k; m<k+mid_square_size; m+=small_square_size) {

                                    const auto pixel_upleft3 = rgb_accessor(work_image.data(),m,n);
                                    evaluate_corners(work_image.data(),m,n,small_square_size);

                                    // do we need smallest resolution -> 3px
                                    bool need_subsampling3 = _compute_corners_heuristic(pixel_upleft3,small_square_size,image_width/*,m,n*/);

                                    if(need_subsampling3) 
                                    {
                                        const auto pixel1 = rgb_accessor(work_image.data(),m+1,n);
                                        const auto pixel2 = rgb_accessor(work_image.data(),m,n+1);
                                        const auto pixel3 = rgb_accessor(work_image.data(),m+1,n+1);
                                        const auto pixel4 = rgb_accessor(work_image.data(),m+2,n+1);
                                        const auto pixel5 = rgb_accessor(work_image.data(),m+1,n+2);
                                        write_color<int>(pixel1, _stochastic_sample(m+1,n), tracer_constants::samples_per_pixel);
                                        write_color<int>(pixel2, _stochastic_sample(m,n+1), tracer_constants::samples_per_pixel);
                                        write_color<int>(pixel3, _stochastic_sample(m+1,n+1), tracer_constants::samples_per_pixel);
                                        write_color<int>(pixel4, _stochastic_sample(m+2,n+1), tracer_constants::samples_per_pixel);
                                        write_color<int>(pixel5, _stochastic_sample(m+1,n+2), tracer_constants::samples_per_pixel);
                                    }
                                    else // interpolate smallest square
                                    {
                                        interpolate_square(work_image.data(),m,n,small_square_size);
                                    }
                                }
                            }
                        }
                        else // interpolate mid square
                        {
                            interpolate_square(work_image.data(),k,l,mid_square_size);
                        }
                    }
                }
            }
            else // interpolate big square
            {
                interpolate_square(work_image.data(),i,j,big_square_size);
            }
        };

        std::mutex mutex;

        const auto run_stripe = [&](int j0, int j1)
        {
            for (int j = j0; j < j1; j+=big_square_size) {
                for (int i = 0; i < image_width; i+=big_square_size) {

                    // process a "big square"
                    process_square(i,j);

                    {
                        std::lock_guard<std::mutex> lock(mutex);

                        // manage dynamic progress gui
                        dgui.show(work_image.data());
                    }
                }
                progress++;
            }
        };

        constexpr int stripe_size = big_square_size*(image_height/(4*big_square_size));
        tp.add_job( [&](){ run_stripe(0,stripe_size); } );
        tp.add_job( [&](){ run_stripe(stripe_size,2*stripe_size); } );
        tp.add_job( [&](){ run_stripe(2*stripe_size,3*stripe_size); } );
        tp.add_job( [&](){ run_stripe(3*stripe_size,image_height); } );
        while(true) {
            const auto percent = 100*progress/(image_height/big_square_size);
            std::cout << "Computing done @" << percent << "%\r" << std::flush;
            std::this_thread::sleep_for(100ms);
            if(percent >= 100)
                break;
        }
        tp.wait_all();        

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
                    write_color(output_image+offset, _stochastic_sample(i,j), tracer_constants::samples_per_pixel);
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

        frame_allocator<float,tracer_constants::frame_size,4> frame_alloc;
        auto& work_image1 = frame_alloc.get_frame(0,0.f);
        auto& work_image2 = frame_alloc.get_frame(1,0.f);
        auto& work_image3 = frame_alloc.get_frame(2,0.f);
        auto& work_image4 = frame_alloc.get_frame(3,0.f);

        auto run_image = [&](float* partial_image,int small_samples_per_pixel) {
            for (int j = 0; j < image_height; ++j) {
                int offset = color_channels*j*image_width;
                for (int i = 0; i < image_width; ++i) {
                    const color pixel_color = _stochastic_sample(i,j,small_samples_per_pixel);
                    auto* out = partial_image+offset;
                    write_color_raw<float>(out,pixel_color); // NOTE: don't apply gamma correction here!
                    offset += color_channels;
                }
                progress++;
            }
        };

        using namespace std::chrono_literals;
        const auto start = std::chrono::steady_clock::now();

        tp.add_job( [&](){ run_image(work_image1.data(), tracer_constants::samples_per_pixel/4); } );
        tp.add_job( [&](){ run_image(work_image2.data(), tracer_constants::samples_per_pixel/4); } );
        tp.add_job( [&](){ run_image(work_image3.data(), tracer_constants::samples_per_pixel/4); } );
        tp.add_job( [&](){ run_image(work_image4.data(), tracer_constants::samples_per_pixel/4); } );
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
                write_color(out, pixel_acc, tracer_constants::samples_per_pixel);
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
};

#endif
