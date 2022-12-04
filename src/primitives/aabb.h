#ifndef AABB_H
#define AABB_H

#include "rtweekend.h"

#include <algorithm>

class aabb {
    public:
        aabb() = default;
        aabb(const point3& a, const point3& b) { minimum = a; maximum = b;}

        point3 min() const {return minimum; }
        point3 max() const {return maximum; }

        inline bool hit(const ray& r, double t_min, double t_max) const {
            for (int a = 0; a < 3; a++) {
                auto invD = 1.0f / r.direction()[a];
                auto t0 = (min()[a] - r.origin()[a]) * invD;
                auto t1 = (max()[a] - r.origin()[a]) * invD;
                if (invD < 0.0f)
                    std::swap(t0, t1);
                t_min = t0 > t_min ? t0 : t_min;
                t_max = t1 < t_max ? t1 : t_max;
                if (t_max <= t_min)
                    return false;
            }
            return true;
        }

        point3 minimum;
        point3 maximum;
};

aabb surrounding_box(aabb box0, aabb box1) {
    point3 small(min(box0.min(),box1.min()));
    point3 big(max(box0.max(),box1.max()));
    return aabb(small,big);
}

#endif
