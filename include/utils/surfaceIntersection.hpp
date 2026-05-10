#include "utils/Material.hpp"
#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include <random>

namespace RayTracer {
    struct SurfaceIntersection
    {
        Math::Vector3D normal;
        Math::Vector3D hitPoint;
        Math::Vector3D rayDir;
        Triangle tri;
        Graphic::Material *mat;
        Graphic::Pixel localColor;
        std::vector<Ray> nextRays;
        Math::Vector3D throughput;
        std::pair<double, double> uv;

        Math::Vector3D getOmega() {
            static thread_local std::mt19937 gen(std::random_device{}());
            static std::uniform_real_distribution<double> dist(0.0, 1.0);

            double r1 = dist(gen);
            double r2 = dist(gen);

            double phi = 2.0 * M_PI * r2;
            double r = std::sqrt(r1); 
            
            double x = r * std::cos(phi);
            double y = r * std::sin(phi);
            double z = std::sqrt(std::max(0.0, 1.0 - r1));

            Math::Vector3D n = normal;
            Math::Vector3D tangent, bitangent;

            if (std::abs(n.z()) < 0.999) {
                tangent = Math::Vector3D(0, 0, 1).cross(n).normalize();
            } else {
                tangent = Math::Vector3D(1, 0, 0).cross(n).normalize();
            }
            bitangent = n.cross(tangent);

            return (tangent * x + bitangent * y + n * z);
        }
    };
}
