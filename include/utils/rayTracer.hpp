#ifndef RAYTRACER_HPP_
#define RAYTRACER_HPP_

#include "math.hpp"

namespace RayTracer {
        
    class Ray {
        public:
            Ray(Math::Vector3D origin, Math::Vector3D direction)
            {
                this->origin = origin;
                this->direction = direction;
            }
            Math::Vector3D origin;
            Math::Vector3D direction;
    };
    
    class Triangle {
        public:
            constexpr Triangle(const Math::Vector3D& a, const Math::Vector3D& b, const Math::Vector3D& c) noexcept : v{a, b, c} {}

            constexpr const Math::Vector3D& a() const noexcept {
                return v[0];
            }
            constexpr const Math::Vector3D& b() const noexcept {
                return v[1];
            }
            constexpr const Math::Vector3D& c() const noexcept {
                return v[2];
            }

            constexpr Math::Vector3D& a() noexcept {
                return v[0];
            }
            constexpr Math::Vector3D& b() noexcept {
                return v[1];
            }
            constexpr Math::Vector3D& c() noexcept {
                return v[2];
            }

            constexpr Math::Vector3D centroid() const noexcept {
                return (v[0] + v[1] + v[2]) * (1.0 / 3.0);
            }

            Math::Vector3D v[3];
    };
}
#endif
