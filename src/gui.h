#ifndef GUI_H
#define GUI_H

#define cimg_display 1 //X11

#include "CImg.h"

using namespace cimg_library;

class gui
{
public:
    static void display( const std::uint8_t* img, int w, int h ) {
        // http://www.cimg.eu/reference/storage.html
        // https://www.codefull.org/2014/11/cimg-does-not-store-pixels-in-the-interleaved-format/
        CImg<std::uint8_t> image(img,3,w,h,1,false);
        image.permute_axes("yzcx");
        CImgDisplay main_disp(image,"Raycaster");
        while (!main_disp.is_closed()) {
            main_disp.wait();
        }
    }
};
#endif
