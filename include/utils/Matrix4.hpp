#ifndef MATRIX4_HPP_
#define MATRIX4_HPP_

#include "opti/BVH/AABB.hpp"
#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
namespace RayTracer {

    class Matrix4 {
        public:
            Matrix4() noexcept : m{
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1},
            } {}

            Matrix4 inverse() const noexcept;

            Math::Vector3D transformPoint(const Math::Vector3D &p) const noexcept;
            Math::Vector3D transformDirection(const Math::Vector3D &d) const noexcept;

            AABB transformAABB(const AABB &box) const noexcept;

            Ray transformRay(const Ray &ray) const noexcept {
                return Ray(transformPoint(ray.origin), transformDirection(ray.direction));
            }
            double m[4][4] = {};
    };

}


#endif