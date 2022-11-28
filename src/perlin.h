#ifndef PERLIN_H
#define PERLIN_H

#include "rtweekend.h"

#include <array>

class perlin {
    public:
        perlin() {
            for (int i = 0; i < point_count; ++i) {
                ranfloat[i] = random_double();
            }

            perlin_generate_perm(perm_x.data());
            perlin_generate_perm(perm_y.data());
            perlin_generate_perm(perm_z.data());
        }

        double noise(const point3& p) const {
            auto u = p.x() - floor(p.x());
            auto v = p.y() - floor(p.y());
            auto w = p.z() - floor(p.z());
            u = u*u*(3-2*u);
            v = v*v*(3-2*v);
            w = w*w*(3-2*w);
            
            auto i = static_cast<int>(floor(p.x()));
            auto j = static_cast<int>(floor(p.y()));
            auto k = static_cast<int>(floor(p.z()));
            double c[2][2][2];

            for (int di=0; di < 2; di++)
                for (int dj=0; dj < 2; dj++)
                    for (int dk=0; dk < 2; dk++)
                        c[di][dj][dk] = ranfloat[
                            perm_x[(i+di) & 255] ^
                            perm_y[(j+dj) & 255] ^
                            perm_z[(k+dk) & 255]
                        ];

            return trilinear_interp(c, u, v, w);
        }

    private:
        constexpr static int point_count = 256;
        std::array<double,point_count> ranfloat;
        std::array<int,point_count> perm_x;
        std::array<int,point_count> perm_y;
        std::array<int,point_count> perm_z;

        static void perlin_generate_perm(int* p) {
            for (int i = 0; i < perlin::point_count; i++)
                p[i] = i;

            permute(p, point_count);
        }

        static void permute(int* p, int n) {
            for (int i = n-1; i > 0; i--) {
                int target = random_int(0, i);
                int tmp = p[i];
                p[i] = p[target];
                p[target] = tmp;
            }
        }
    
        static double trilinear_interp(double c[2][2][2], double u, double v, double w) {
            auto accum = 0.0;
            for (int i=0; i < 2; i++)
                for (int j=0; j < 2; j++)
                    for (int k=0; k < 2; k++)
                        accum += (i*u + (1-i)*(1-u))*
                                (j*v + (1-j)*(1-v))*
                                (k*w + (1-k)*(1-w))*c[i][j][k];

            return accum;
        }
};

#endif
