#ifndef RAYTRACER_HPP_
#define RAYTRACER_HPP_

#include "math.hpp"

namespace RayTracer {
        
    struct Ray {
        public:
            Ray(Math::Vector3D ro,
                Math::Vector3D rd,
                Math::Vector3D throughput = {1.0, 1.0, 1.0},
                int depth = 0)
            {
                this->origin = ro;
                this->direction= rd;
                this->throughput = throughput;
                this->depth = depth;
            }
            Math::Vector3D origin;
            Math::Vector3D direction;
            Math::Vector3D throughput;
            int depth;
    };
    
    class Triangle {
        public:
            constexpr Triangle(const Math::Vector3D& a, const Math::Vector3D& b, const Math::Vector3D& c) noexcept : v{a, b, c} {}
            constexpr Triangle() noexcept : v{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}} {}

            constexpr const Math::Vector3D& a() const noexcept { return v[0]; }
            constexpr const Math::Vector3D& b() const noexcept { return v[1]; }
            constexpr const Math::Vector3D& c() const noexcept { return v[2]; }

            constexpr Math::Vector3D& a() noexcept { return v[0]; }
            constexpr Math::Vector3D& b() noexcept { return v[1]; }
            constexpr Math::Vector3D& c() noexcept { return v[2]; }

            constexpr Math::Vector3D centroid() const noexcept {
                return (v[0] + v[1] + v[2]) * (1.0 / 3.0);
            }

            Math::Vector3D v[3];
            Math::Vector3D vn[3];
            std::pair<double, double> uv[3];
            Math::Vector3D normal;
    };
}

#endif