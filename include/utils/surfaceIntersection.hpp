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
        Graphic::Material *mat;
        Graphic::Pixel localColor;
        RayTracer::Ray nextRay;
        bool shouldBounce;
        Math::Vector3D throughput;

        Math::Vector3D getOmega() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<double> dist(0.0, 1.0);

            double r1 = dist(gen);
            double r2 = dist(gen);

            double r = std::sqrt(r1);
            double phi = 2 * M_PI * r2;

            double x = r * std::cos(phi);
            double y = r * std::sin(phi);
            double z = std::sqrt(1 - r1);

            // Build coordinate system from normal
            Math::Vector3D n = normal;
            Math::Vector3D ref = (std::abs(n.z()) < 0.999) ? Math::Vector3D(0, 0, 1) : Math::Vector3D(1, 0, 0);
            Math::Vector3D tangent = n.cross(ref).normalize();
            Math::Vector3D bitangent = n.cross(tangent);

            return (tangent * x + bitangent * y + n * z).normalize();
        }
    };
}
