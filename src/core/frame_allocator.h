#ifndef FRAME_ALLOCATOR_H
#define FRAME_ALLOCATOR_H

#include "tracer_constants.h"

template<typename T, size_t frame_size, size_t stock_size>
class frame_allocator_stack {
    public:

        using tracer_frame = std::array<T,frame_size>;

        tracer_frame& get_frame(size_t index)
        {
            // TODO-AM : index static check
            return output_frame[index];
        }

    private:
        std::array<tracer_frame,stock_size> output_frame;
};

template<typename T, size_t frame_size>
class frame_allocator_heap {
    public:

        using tracer_frame = std::array<T,frame_size>;

        tracer_frame get_frame(size_t index)
        {
            return tracer_frame{};
        }
};

// conditional derivation:
// https://stackoverflow.com/questions/71772455/can-c-class-attributes-be-conditionally-compiled
template<typename T, size_t frame_size, size_t stock_size>
class frame_allocator : public std::conditional_t<tracer_constants::stack_alloc,
    frame_allocator_stack<T,frame_size,stock_size>,
    frame_allocator_heap<T,frame_size>> {

};

#endif
