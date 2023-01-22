#ifndef PDF_H
#define PDF_H

#include "tracer_utils.h"

class pdf {
    public:
        virtual ~pdf() = default;

        virtual double value(const vec3& direction) const = 0;
        virtual vec3 generate() const = 0;
};

class cosine_pdf : public pdf {
    public:
        explicit cosine_pdf(const vec3& w) { uvw.build_from_w(w); }

        virtual double value(const vec3& direction) const override {
            auto cosine = dot(unit_vector(direction), uvw.w());
            return (cosine <= 0) ? 0 : cosine/pi;
        }

        virtual vec3 generate() const override {
            return uvw.local(random_cosine_direction());
        }

    public:
        onb uvw;
};

class hittable_pdf : public pdf {
    public:
        hittable_pdf(const hittable& p, const point3& origin) : pref(p), o(origin) {}

        virtual double value(const vec3& direction) const override {
            return pref.pdf_value(o, direction);
        }

        virtual vec3 generate() const override {
            return pref.random(o);
        }

    public:
        const hittable& pref;
        point3 o;
};

class mixture_pdf : public pdf {
    public:
        mixture_pdf(std::shared_ptr<pdf> p0, std::shared_ptr<pdf> p1) {
            p[0] = p0;
            p[1] = p1;
        }

        virtual double value(const vec3& direction) const override {
            return 0.5 * p[0]->value(direction) + 0.5 *p[1]->value(direction);
        }

        virtual vec3 generate() const override {
            if (random_double() < 0.5)
                return p[0]->generate();
            else
                return p[1]->generate();
        }

    public:
        std::shared_ptr<pdf> p[2];
};

#endif