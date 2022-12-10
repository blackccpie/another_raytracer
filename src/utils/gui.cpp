#include "gui.h"

#ifdef WIN32
    #define cimg_display 2 //Windows-GDI
#else
    #define cimg_display 1 //X11
#endif

#include "CImg.h"

using namespace cimg_library;

void gui::display( const std::uint8_t* img, int w, int h ) {
    // http://www.cimg.eu/reference/storage.html
    // https://www.codefull.org/2014/11/cimg-does-not-store-pixels-in-the-interleaved-format/
    CImg<std::uint8_t> image(img,3,static_cast<std::uint32_t>(w),static_cast<std::uint32_t>(h),1,false);
    image.permute_axes("yzcx");
    CImgDisplay main_disp(image,"Raycaster");
    while (!main_disp.is_closed()) {
        main_disp.wait();
    }
}
