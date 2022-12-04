#ifndef IMAGEIO_H
#define IMAGEIO_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class imageio
{
public:
    static std::unique_ptr<unsigned char[]> load_image( const std::string& path, int& width, int& height, int& bytes_per_pixel )
    {
        return std::unique_ptr<unsigned char[]>( stbi_load(
            path.c_str(), &width, &height, &bytes_per_pixel, 0) );
    }
    
    static bool save_image( const std::string& path, int width, int height, int bytes_per_pixel, const void *data )
    {
        return stbi_write_png(path.c_str(), width, height, bytes_per_pixel, data, 0) != 0;
    }
};

#endif
