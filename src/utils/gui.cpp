#include "gui.h"

#ifdef WIN32
    #define cimg_display 2 //Windows-GDI
#else
    #define cimg_display 1 //X11
#endif

#include "CImg.h"

using namespace cimg_library;

void gui::display( const std::uint8_t* img, int w, int h, int scale ) {
    // http://www.cimg.eu/reference/storage.html
    // https://www.codefull.org/2014/11/cimg-does-not-store-pixels-in-the-interleaved-format/
    CImg<std::uint8_t> image(img,3,static_cast<std::uint32_t>(w),static_cast<std::uint32_t>(h),1,false);
    image.permute_axes("yzcx");
    CImgDisplay main_disp(image,"Raycaster");
    main_disp.resize(scale*w,scale*h);
    while (!main_disp.is_closed()) {
        main_disp.wait();
    }
}

dynamic_gui::dynamic_gui(int w, int h, int scale, const std::string& title) 
    : width(w), height(h),
    display( std::make_unique<CImgDisplay>(scale*w,scale*h,title.c_str()) ),
    dthread( &dynamic_gui::_display_thread, this )
{
}

dynamic_gui::~dynamic_gui()
{
    display->close();
    dthread.join();
}

template<typename T>
void dynamic_gui::show(const T* img)
{
    // http://www.cimg.eu/reference/storage.html
    // https://www.codefull.org/2014/11/cimg-does-not-store-pixels-in-the-interleaved-format/
    CImg<T> image(img,3,static_cast<std::uint32_t>(width),static_cast<std::uint32_t>(height),1,false);
    image.permute_axes("yzcx");
    display->display(image);
}

template void dynamic_gui::show<std::uint8_t>(const std::uint8_t* img);
template void dynamic_gui::show<std::int32_t>(const std::int32_t* img);

void dynamic_gui::_display_thread()
{
    while (!display->is_closed()) {
        display->wait(100);
    }
}
