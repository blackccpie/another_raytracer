#ifndef GUI_H
#define GUI_H

#include <cstdint>
#include <memory>
#include <thread>

namespace cimg_library{ class CImgDisplay; }

class gui
{
public:
    static void display( const std::uint8_t* img, int w, int h, int scale );
};

class dynamic_gui
{
public:
    dynamic_gui(int w, int h, int scale, const std::string& title);
    ~dynamic_gui();
    template<typename T>
    void show(const T* img);
private:
    void _display_thread();
private:
    int width = 0;
    int height = 0;
    std::unique_ptr<cimg_library::CImgDisplay> display;
    std::thread dthread;
};

#endif
