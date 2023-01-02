#ifndef TEXTURE_H
#define TEXTURE_H

#include "imageio.h"
#include "perlin.h"
#include "rtweekend.h"
//#include "gui.h"

#include <string>

class texture {
    public:
        virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color final : public texture {
    public:
        solid_color(color c) : color_value(c) {}

        solid_color(double red, double green, double blue)
          : solid_color(color(red,green,blue)) {}

        virtual color value(double u, double v, const vec3& p) const override {
            return color_value;
        }

    private:
        color color_value;
};

class checker_texture final : public texture {
    public:
        checker_texture(std::shared_ptr<texture> _even, std::shared_ptr<texture> _odd)
            : even(_even), odd(_odd) {}

        checker_texture(color c1, color c2)
            : even(std::make_shared<solid_color>(c1)) , odd(std::make_shared<solid_color>(c2)) {}

        virtual color value(double u, double v, const point3& p) const override {
            auto sines = sin(10*p.x())*sin(10*p.y())*sin(10*p.z());
            if (sines < 0)
                return odd->value(u, v, p);
            else
                return even->value(u, v, p);
        }

    public:
        std::shared_ptr<texture> even;
        std::shared_ptr<texture> odd;
};

class noise_texture final : public texture {
    public:

        noise_texture(double sc) : scale(sc) {}

        virtual color value(double u, double v, const point3& p) const override {
            return color(1,1,1) * 0.5 * (1.0 + noise.noise(scale * p));
            //return color(1,1,1) * 0.5 * (1 + sin(scale*p.z() + 10*noise.turb(p)));
        }

    public:
        perlin noise;
        double scale;
};

class image_texture final : public texture {
    public:

        image_texture(const std::string& filename) {
            
            data = imageio::load_image( filename, width, height, bytes_per_pixel);

            if (!data) {
                std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
                width = height = 0;
            }
            else
            {
                std::cout << "successfully loaded texture image (" << width << "x" << height << "x" << bytes_per_pixel << ")" << std::endl;
            }

            bytes_per_scanline = bytes_per_pixel * width;
            
            //gui::display( data.get(), width, height );
        }

        virtual color value(double u, double v, const point3& p) const override {
            
            // If we have no texture data, then return solid cyan as a debugging aid.
            if (data == nullptr)
                return color(0,1,1);
            
            // Clamp input texture coordinates to [0,1] x [1,0]
            u = clamp(u, 0.0, 1.0);
            v = 1.0 - clamp(v, 0.0, 1.0);  // Flip V to image coordinates

            auto i = static_cast<int>(u * width);
            auto j = static_cast<int>(v * height);

            // Clamp integer mapping, since actual coordinates should be less than 1.0
            if (i >= width)  i = width-1;
            if (j >= height) j = height-1;

            //std::cout << "ij " << i << " " << j << std::endl;

            const auto color_scale = 1.0 / 255.0;
            auto pixel = data.get() + j*bytes_per_scanline + i*bytes_per_pixel;
            
            return color(color_scale*pixel[0], color_scale*pixel[1], color_scale*pixel[2]);
        }

    private:

        std::unique_ptr<unsigned char[]> data;
        int width = 0, height = 0, bytes_per_pixel = 0;
        int bytes_per_scanline = 0;
};

// TODO-AM : not sure this texturing approach will be usefull...
class barycentric_texture final : public texture {
    public:
        barycentric_texture(color a, color b, color c) : color_a(a), color_b(b), color_c(c) {}

        virtual color value(double u, double v, const vec3& p) const override {
            return u*color_a + v*color_b + (1.0-u-v)*color_c;
        }

    private:
        color color_a;
        color color_b;
        color color_c;
};

class barycentric_image_texture final : public texture {
    public:
        using uv = std::pair<double,double>;
        barycentric_image_texture(uv a, uv b, uv c, std::shared_ptr<image_texture> _tex) 
            : texcoord_a(a), texcoord_b(b), texcoord_c(c), tex(_tex) {}

        virtual color value(double u, double v, const vec3& p) const override {
            return tex->value(
                u * texcoord_a.first + v * texcoord_b.first + (1-u-v) * texcoord_c.first,
                u * texcoord_a.second + v * texcoord_b.second + (1-u-v) * texcoord_c.second,
                vec3{}
            );
        }

    private:
        uv texcoord_a;
        uv texcoord_b;
        uv texcoord_c;
        std::shared_ptr<image_texture> tex;
};

#endif
