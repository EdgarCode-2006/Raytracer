#include "integrators/Integrator.hpp"

namespace RayTracer
{
    static void integrateColor(
        Math::Vector3D &finalColor,
        const Math::Vector3D &throughput,
        const Graphic::Pixel &localColor,
        const float emission = 0.0
    )
    {
        finalColor.x() += (localColor._r) * (throughput.x() + emission);
        finalColor.y() += (localColor._g) * (throughput.y() + emission);
        finalColor.z() += (localColor._b) * (throughput.z() + emission);
    }

    static void initializeIntersection(
        SurfaceIntersection &intersection,
        Graphic::Material *mat,
        Math::Vector3D hitPoint,
        Math::Vector3D normal,
        Math::Vector3D rayDir
    )
    {
        intersection.mat = mat;
        intersection.hitPoint = hitPoint;
        intersection.normal = normal;
        intersection.rayDir = rayDir;
        intersection.localColor = {0, 0, 0, 255};
    }

    static void instanceNewRays(std::vector<Ray> &nextRays, Ray &currentRayData, std::queue<Ray> &rayQueue)
    {
        for (auto &rayData : nextRays) {
            Ray newRay = {
                rayData.origin,
                rayData.direction,
                rayData.throughput,
                currentRayData.depth + 1,
            };
            rayQueue.push(newRay);
        }
        nextRays.clear();
    }

    Graphic::Pixel Integrator::consumeRays(Ray &ray, std::shared_ptr<const RenderScene> scene, SurfaceIntersection &intersection)
    {
        std::queue<Ray> rayQueue;
        Math::Vector3D finalColor = {0.0, 0.0, 0.0};
        rayQueue.push(ray); // initial ray

        while (!rayQueue.empty()) {
            Ray currentRay = rayQueue.front();
            rayQueue.pop();

            if (currentRay.depth >= MAX_DEPTH || currentRay.throughput.length() < MIN_THROUGHPUT_LENGTH)
                continue;

            const TLASHit tlasHit = scene->tlas->traverse(currentRay);
            if (!tlasHit.hasHit()) {
                const Graphic::Pixel envPixel = scene->lights.getHDRIPixel(currentRay.direction * -1);
                integrateColor(finalColor, currentRay.throughput, envPixel);
                break;
            }
            const Instance &instance = scene->world.getInstance(tlasHit.instanceIndex());
            const Triangle &tri = instance.getPrimitive(tlasHit.primitiveIndex());

            initializeIntersection(intersection,
                const_cast<Graphic::Material*>(&instance.getMaterial()),
                currentRay.origin + currentRay.direction * tlasHit.t(),
                (tri.b() - tri.a()).cross(tri.c() - tri.a()).normalize(),
                currentRay.direction);
            intersection.throughput = currentRay.throughput;

            for (auto &rayFeature : _pipeline) {
                if (rayFeature->isActive())
                    rayFeature->process(scene, intersection);
            }

            integrateColor(finalColor, currentRay.throughput, intersection.localColor);
            instanceNewRays(intersection.nextRays, currentRay, rayQueue);
        }
        return {
            (unsigned char)std::min(255.0, finalColor.x()),
            (unsigned char)std::min(255.0, finalColor.y()),
            (unsigned char)std::min(255.0, finalColor.z()),
            255
        };
    }

    Graphic::Pixel Integrator::shootRay(Ray &ray, std::shared_ptr<const RenderScene> sceneLocked)
    {
        SurfaceIntersection intersection{
            .normal = {0, 0, 0},
            .hitPoint = {0, 0, 0},
            .rayDir = ray.direction,
            .localColor = {0, 0, 0, 255},
            .throughput = {1.0, 1.0, 1.0}
        };
        return consumeRays(ray, sceneLocked, intersection);
    }

    // --- Old implementation (kept for reference) ---
    // Graphic::Pixel Integrator::shootRay(Ray &ray)
    // {
    //     Math::Vector3D finalColor = {0, 0, 0};
    //     Math::Vector3D throughput = {1.0, 1.0, 1.0};
    //
    //     SurfaceIntersection intersection{
    //         .normal = {0, 0, 0},
    //         .hitPoint = {0, 0, 0},
    //         .rayDir = ray.direction,
    //         .localColor = {0, 0, 0, 255},
    //         .nextRay = ray,
    //         .shouldBounce = false,
    //         .throughput = {1.0, 1.0, 1.0}
    //     };
    //
    //     for (size_t i = 0; i < MAX_DEPTH; i++) {
    //         const TLASHit tlasHit = _scene.getTLAS().traverse(ray);
    //         if (!tlasHit.hasHit()) {
    //             const Graphic::Pixel envPixel = data.scene.getHDRIPixel(ray.direction * -1);
    //             integrateColor(finalColor, throughput, envPixel);
    //             break;
    //         }
    //         const Instance &instance = _scene.getInstance(tlasHit.instanceIndex());
    //         intersection.mat = const_cast<Graphic::Material*>(&instance.getMaterial());
    //         const Triangle &tri = instance.getPrimitive(tlasHit.primitiveIndex());
    //
    //         intersection.hitPoint = ray.origin + ray.direction * tlasHit.t();
    //         intersection.normal = (tri.b() - tri.a()).cross(tri.c() - tri.a()).normalize();
    //         intersection.rayDir = ray.direction;
    //         intersection.shouldBounce = false;
    //
    //         for (auto &rayFeature : _pipeline) {
    //             if (rayFeature->isActive())
    //                 rayFeature->process(data, intersection);
    //         }
    //
    //         integrateColor(finalColor, throughput, intersection.localColor);
    //
    //         if (!intersection.shouldBounce) break;
    //
    //         throughput.x() *= intersection.throughput.x();
    //         throughput.y() *= intersection.throughput.y();
    //         throughput.z() *= intersection.throughput.z();
    //
    //         ray = intersection.nextRay;
    //
    //         if (throughput.length() < MIN_THROUGHPUT_LENGTH) break;
    //     }
    //     return {
    //         (unsigned char)std::min(255.0, finalColor.x()),
    //         (unsigned char)std::min(255.0, finalColor.y()),
    //         (unsigned char)std::min(255.0, finalColor.z()),
    //         255
    //     };
    // }
} // namespace RayTracer
