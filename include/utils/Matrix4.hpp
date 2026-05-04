#ifndef MATRIX4_HPP_
#define MATRIX4_HPP_

#include "opti/BVH/AABB.hpp"
#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
namespace Math {

    class Matrix4{
        public:
            Matrix4() noexcept : m{
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1},
            } {}

            Matrix4 inverse() const noexcept;

            Matrix4 translation(double tx, double ty, double tz) noexcept;
            Matrix4 scale(double sx, double sy, double sz) noexcept;
            Matrix4 rotationX(double rad) noexcept;
            Matrix4 rotationY(double rad) noexcept;
            Matrix4 rotationZ(double rad) noexcept;
            

            Vector3D transformPoint(const Vector3D &p) const noexcept;
            Vector3D transformDirection(const Vector3D &d) const noexcept;
            Matrix4 operator*(const Matrix4& o) const noexcept;
            AABB transformAABB(const AABB &box) const noexcept;

            RayTracer::Ray transformRay(const RayTracer::Ray &ray) const noexcept {
                return RayTracer::Ray(transformPoint(ray.origin), transformDirection(ray.direction));
            }
            double m[4][4] = {};
    };

}


#endif