#ifndef FRAME_ALLOCATOR_H
#define FRAME_ALLOCATOR_H

#include "tracer_constants.h"

template<typename T, size_t frame_size, size_t stock_size>
class frame_allocator_stack {
    public:

        using tracer_frame = std::array<T,frame_size>;

        frame_allocator_stack(){ std::cout << "--> using frame allocator on stack memory"<< std::endl; }

        tracer_frame& get_frame(size_t index, T default_value)
        {
            if(index >= stock_size) throw std::logic_error("invalid frame index!");
            output_frames[index].fill(default_value);
            return output_frames[index];
        }
    
    private:
        /*constexpr auto make_array(T value) -> std::array<T,frame_size>
        {
            std::array<T,frame_size> a{};
            a.fill(value);
            return a;
        }*/

    private:
        std::array<tracer_frame,stock_size> output_frames;// = make_array(0);
};

template<typename T, size_t frame_size, size_t stock_size>
class frame_allocator_heap {
    public:

        using tracer_frame = std::array<T,frame_size>;

        frame_allocator_heap(){ std::cout << "--> using frame allocator on heap memory"<< std::endl; }

        tracer_frame& get_frame(size_t index, T default_value)
        {
            if(index >= stock_size) throw std::logic_error("invalid frame index!");
            output_frames[index] = std::make_unique<tracer_frame>();
            output_frames[index]->fill(default_value);
            return *output_frames[index];
        }
    private:
        std::array<std::unique_ptr<tracer_frame>,stock_size> output_frames;
};

// conditional derivation:
// https://stackoverflow.com/questions/71772455/can-c-class-attributes-be-conditionally-compiled
template<typename T, size_t frame_size, size_t stock_size>
class frame_allocator : public std::conditional_t<tracer_constants::stack_alloc,
    frame_allocator_stack<T,frame_size,stock_size>,
    frame_allocator_heap<T,frame_size,stock_size>> {
    public:
        frame_allocator() {
            std::cout << "--> frame allocator provisioning " << (stock_size*frame_size/1024) << "kb of frame memory (" << stock_size << " frames )" << std::endl;
        }
};

#endif
