#ifndef HELPER_HPP_
#define HELPER_HPP_
#include "utils/math.hpp"
#include "utils/rayTracer.hpp"

namespace RayTracer::Utils {
    inline double intersectTriangle(const Ray& ray, const Triangle& tri)
    {
        const double EPSILON = 1e-7;
        Math::Vector3D edge1 = tri.b() - tri.a();
        Math::Vector3D edge2 = tri.c() - tri.a();
        Math::Vector3D h = ray.direction.cross(edge2);
        double a = edge1.dot(h);
        if (a > -EPSILON && a < EPSILON)
            return -1;
        double f = 1.0 / a;
        Math::Vector3D s = ray.origin - tri.a();
        double u = f * s.dot(h);
        if (u < 0.0 || u > 1.0)
            return -1;
        Math::Vector3D q = s.cross(edge1);
        double v = f * ray.direction.dot(q);
        if (v < 0.0 || u + v > 1.0)
            return -1;
        double t = f * edge2.dot(q);
        return (t > EPSILON) ? t : -1;
    }
}
#endif