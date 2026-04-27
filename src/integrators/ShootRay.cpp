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
            .scene = this->_scene
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
            const TLASHit tlasHit = _scene.getTLAS().traverse(ray);
            if (!tlasHit.hasHit())
                break;
            const Instance &instance = _scene.getInstance(tlasHit.instanceIndex());
            const Triangle &tri = instance.getPrimitive(tlasHit.primitiveIndex());

            intersection.hitPoint = ray.origin + ray.direction * tlasHit.t();
            intersection.normal = (tri.b() - tri.a()).cross(tri.c() - tri.a()).normalize();
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
