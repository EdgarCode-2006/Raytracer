#include "integrators/Integrator.hpp"

namespace RayTracer
{
    Graphic::Pixel Integrator::shootRay(Ray &ray)
    {
        Math::Vector3D finalColor = {0, 0, 0};
        Math::Vector3D throughput = {1.0, 1.0, 1.0};

        RenderData data{
            .lights = this->_scene.getLights(),
            .ambientLight = this->_scene.getAmbientLight(),
            .scene = this->_scene,
        };
        SurfaceIntersection intersection{
            .normal = {0, 0, 0},
            .hitPoint = {0, 0, 0},
            .rayDir = ray.direction,
            .localColor = {0, 0, 0, 255},
            .nextRay = ray,
            .shouldBounce = false,
            .throughput = {1.0, 1.0, 1.0}
        };

        for (size_t i = 0; i < MAX_DEPTH; i++) {
            double closest_t = 1e9;
            const Triangle* hit = nullptr;

            for (const auto& tri : data.scene.getTriangles()) {
                double t = Utils::intersectTriangle(ray, tri);
                if (t > 0 && t < closest_t) {
                    closest_t = t;
                    hit = &tri;
                }
            }

            if (!hit)
                break;
            intersection.hitPoint = ray.origin + ray.direction * closest_t;
            intersection.normal = (hit->b() - hit->a()).cross(hit->c() - hit->a()).normalize();
            intersection.rayDir = ray.direction;
            intersection.shouldBounce = false;

            for (auto &rayFeature : _pipeline) {
                if (rayFeature->isActive())
                    rayFeature->process(data, intersection);
            }

            finalColor.x() += (intersection.localColor._r) * throughput.x();
            finalColor.y() += (intersection.localColor._g) * throughput.y();
            finalColor.z() += (intersection.localColor._b) * throughput.z();

            if (!intersection.shouldBounce) break;

            throughput.x() *= intersection.throughput.x();
            throughput.y() *= intersection.throughput.y();
            throughput.z() *= intersection.throughput.z();

            ray = intersection.nextRay;

            if (throughput.length() < MIN_THROUGHPUT_LENGTH) break;
        }
        return {
            (unsigned char)std::min(255.0, finalColor.x()),
            (unsigned char)std::min(255.0, finalColor.y()),
            (unsigned char)std::min(255.0, finalColor.z()),
            255
        };
    }
} // namespace RayTracer
