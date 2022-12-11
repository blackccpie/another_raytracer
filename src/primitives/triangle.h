#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "vec3.h"

class triangle final : public hittable {
    public:
        triangle(point3 _pt1, point3 _pt2, point3 _pt3, shared_ptr<material> m)
            : pt1(_pt1), pt2(_pt2), pt3(_pt3), mat_ptr(m) {}

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

    public:
        point3 pt1;
        point3 pt2;
        point3 pt3;
        shared_ptr<material> mat_ptr;
};

bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    
    // INSPIRED BY:
    // www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates
    
    // compute plane's normal
    vec3 v1v2 = pt2 - pt1;
    vec3 v1v3 = pt3 - pt1;
    // no need to normalize
    vec3 outward_normal = cross(v1v2,v1v3);
    
    // Step 1: finding P
     
    // check if ray and plane are parallel.
    double normal_dot_ray_direction = dot(outward_normal,r.direction());
    if (std::fabs(normal_dot_ray_direction) < std::numeric_limits<double>::epsilon()) // TODO-AM: really a proper use of epsilon here?
        return false; //they are parallel so they don't intersect !
     
    // compute d parameter using equation 2
    double d = -dot(outward_normal,pt1);
     
    // compute t (equation 3)
    double t = -(dot(outward_normal,r.origin()) + d) / normal_dot_ray_direction;
 
    // check if the triangle is in behind the ray (disabled because redundant with the upcomoing range test)
    //if (t < 0) return false; //the triangle is behind
 
    // Check t lies in the acceptable range.
    if (t < t_min || t_max < t)
        return false;
    
    // compute the intersection point using equation 1
    vec3 p = r.origin() + t * r.direction();
 
    // Step 2: inside-outside test
    
    double u,v;
    vec3 c; //vector perpendicular to triangle's plane
 
    // edge 1
    vec3 edge1 = pt2 - pt1;
    vec3 vpt1 = p - pt1;
    c = cross(edge1,vpt1);
    if (dot(outward_normal,c) < 0) return false; // p is on the right (out) side
 
    // edge 2
    vec3 edge2 = pt3 - pt2;
    vec3 vpt2 = p - pt2;
    c = cross(edge2,vpt2);
    if ((u=dot(outward_normal,c)) < 0)  return false; // p is on the right (out) side
 
    // edge 3
    vec3 edge3 = pt1 - pt3;
    vec3 vpt3 = p - pt3;
    c = cross(edge3,vpt3);
    if ((v=dot(outward_normal,c)) < 0) return false; // p is on the right (out) side
 
    rec.t = t;
    rec.p = p;
    rec.set_face_normal(r, outward_normal);
    rec.u = u/outward_normal.length();
    rec.v = v/outward_normal.length();
    rec.mat_ptr = mat_ptr;
    
    return true;
}

bool triangle::bounding_box(double time0, double time1, aabb& output_box) const {
    output_box = aabb(
          min(pt1,min(pt2,pt3)),
          max(pt1,max(pt2,pt3)));
    return true;
}

#endif
