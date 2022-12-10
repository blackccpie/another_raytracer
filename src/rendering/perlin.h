#ifndef PERLIN_H
#define PERLIN_H

#include "rtweekend.h"

#include <array>

class perlin {
    public:
        perlin() {
            for( auto& v : ranvec )
            {
                v = unit_vector(vec3::random(-1,1));
            }

            perlin_generate_perm(perm_x.data());
            perlin_generate_perm(perm_y.data());
            perlin_generate_perm(perm_z.data());
        }

        double noise(const point3& p) const {
            auto u = p.x() - floor(p.x());
            auto v = p.y() - floor(p.y());
            auto w = p.z() - floor(p.z());
            auto i = static_cast<int>(floor(p.x()));
            auto j = static_cast<int>(floor(p.y()));
            auto k = static_cast<int>(floor(p.z()));
            vec3 c[2][2][2];

            for (int di=0; di < 2; di++)
                for (int dj=0; dj < 2; dj++)
                    for (int dk=0; dk < 2; dk++)
                        c[di][dj][dk] = ranvec[
                            perm_x[(i+di) & 255] ^
                            perm_y[(j+dj) & 255] ^
                            perm_z[(k+dk) & 255]
                        ];

            return perlin_interp(c, u, v, w);
        }
    
        double turb(const point3& p, int depth=7) const {
            auto accum = 0.0;
            auto temp_p = p;
            auto weight = 1.0;

            for (int i = 0; i < depth; i++) {
                accum += weight*noise(temp_p);
                weight *= 0.5;
                temp_p *= 2;
            }

            return fabs(accum);
        }

    private:
        constexpr static int point_count = 256;
        std::array<vec3,point_count> ranvec;
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
    
        static double perlin_interp(vec3 c[2][2][2], double u, double v, double w) {
            auto uu = u*u*(3-2*u);
            auto vv = v*v*(3-2*v);
            auto ww = w*w*(3-2*w);
            auto accum = 0.0;

            for (int i=0; i < 2; i++)
                for (int j=0; j < 2; j++)
                    for (int k=0; k < 2; k++) {
                        vec3 weight_v(u-i, v-j, w-k);
                        accum += (i*uu + (1-i)*(1-uu))
                               * (j*vv + (1-j)*(1-vv))
                               * (k*ww + (1-k)*(1-ww))
                               * dot(c[i][j][k], weight_v);
                    }

            return accum;
        }
};

#endif
