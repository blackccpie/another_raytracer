#ifndef GUI_H
#define GUI_H

#include "tracer_constants.h"

#include <cstdint>
#include <memory>
#include <thread>

namespace cimg_library{ class CImgDisplay; }

class gui
{
public:
    static void display( const std::uint8_t* img, int w, int h, int scale );
};

class dynamic_gui_impl
{
public:
    dynamic_gui_impl(int w, int h, int scale, const std::string& title);
    ~dynamic_gui_impl();
    template<typename T>
    void show(const T* img);
private:
    void _display_thread();
private:
    int width = 0;
    int height = 0;
    std::unique_ptr<cimg_library::CImgDisplay> display;
    std::thread dthread;
    std::mutex mutex;
};

class dynamic_gui_stub
{
public:
    dynamic_gui_stub(int w, int h, int scale, const std::string& title) {}
    template<typename T> void show(const T* img) {}
};

using my_dynamic_gui_t = std::conditional_t<tracer_constants::progress_gui,dynamic_gui_impl,dynamic_gui_stub>;

// conditional derivation:
// https://stackoverflow.com/questions/71772455/can-c-class-attributes-be-conditionally-compiled
class dynamic_gui : public my_dynamic_gui_t
{
    public:
        dynamic_gui(int w, int h, int scale, const std::string& title) : my_dynamic_gui_t(w,h,scale,title) {}
};

#endif
