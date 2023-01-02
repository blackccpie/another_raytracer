#ifndef IMAGEIO_H
#define IMAGEIO_H

#include <memory>

class imageio
{
public:
    static std::unique_ptr<unsigned char[]> load_image( const std::string& path, int& width, int& height, int& bytes_per_pixel );
    static bool save_image( const std::string& path, int width, int height, int bytes_per_pixel, const void *data );
};

#endif
