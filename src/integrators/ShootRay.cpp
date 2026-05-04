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

        // loop until the queue is done.
        while (!rayQueue.empty()) {
            Ray currentRay = rayQueue.front();
            rayQueue.pop();

            // check if the rays is at max depth or still has energy.
            if (currentRay.depth >= MAX_DEPTH || currentRay.throughput.length() < MIN_THROUGHPUT_LENGTH)
                continue;

            // check for collision using the TLAS.
            const TLASHit tlasHit = scene->tlas->traverse(currentRay);
            if (!tlasHit.hasHit()) {
                const Graphic::Pixel envPixel = scene->lights.getHDRIPixel(currentRay.direction * -1);
                integrateColor(finalColor, currentRay.throughput, envPixel);
                break;
            }
            const Instance &instance = scene->world.getInstance(tlasHit.instanceIndex());
            const Triangle &tri = instance.getPrimitive(tlasHit.primitiveIndex());
    
            // initialize the intersection struct with the new TLAS data.
            initializeIntersection(intersection,
                const_cast<Graphic::Material*>(&instance.getMaterial()),
                currentRay.origin + currentRay.direction * tlasHit.t(),
                (tri.b() - tri.a()).cross(tri.c() - tri.a()).normalize(),
                currentRay.direction);
            intersection.throughput = currentRay.throughput;

            // execute the plugin pipeline which
            // can potentially add to the currentQUeue of rays.
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
} // namespace RayTracer
