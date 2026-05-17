#include "integrators/Integrator.hpp"
#include <iostream>

namespace RayTracer
{
    static void integrateColor(
        Math::Vector3D &finalColor,
        const Math::Vector3D &throughput,
        const Graphic::Pixel &localColor,
        const float emission = 0.0
    )
    {
        finalColor.x() += (localColor._r) * throughput.x();
        finalColor.y() += (localColor._g) * throughput.y();
        finalColor.z() += (localColor._b) * throughput.z();
    }

    static void initializeIntersection(
        SurfaceIntersection &intersection,
        Graphic::Material *mat,
        const Math::Vector3D &hitPoint,
        const Triangle &triangle,
        const Math::Vector3D &normal,
        const Math::Vector3D &rayDir,
        const TLASHit &tlasHit
    )
    {
        intersection.mat = mat;
        intersection.hitPoint = hitPoint;
        intersection.tri = triangle;
        intersection.normal = normal;
        intersection.rayDir = rayDir;
        intersection.localColor = {0, 0, 0, 255};
        intersection.tlasHit = tlasHit;
    }

    static void instanceNewRays(std::vector<Ray> &nextRays, std::queue<Ray> &rayQueue, const Ray &currentRayData = {})
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

    void Integrator::executePipeline(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &intersection, ShaderType type)
    {
        for (auto &shaderIndex : this->_pipelineManager->_shaderMap[type]) {
            auto &shader = this->_pipelineManager->_pipeline[shaderIndex]; 
            if (shader->isActive())
                shader->process(scene, intersection);
        }
    }

    Graphic::Pixel Integrator::shootRay(SurfaceIntersection &intersection, std::shared_ptr<const RenderScene> scene)
    {
        std::queue<Ray> rayQueue;
        Math::Vector3D finalColor = {0.0, 0.0, 0.0};
        instanceNewRays(intersection.nextRays, rayQueue);

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
                tri,
                tri.normal,
                currentRay.direction,
                tlasHit);
            intersection.throughput = currentRay.throughput;

            this->executePipeline(scene, intersection, UTILITY);
            this->executePipeline(scene, intersection, intersection.mat->type);

            integrateColor(finalColor, currentRay.throughput, intersection.localColor);
            instanceNewRays(intersection.nextRays, rayQueue, currentRay);
        }
        return {
            (unsigned char)std::min(255.0, finalColor.x()),
            (unsigned char)std::min(255.0, finalColor.y()),
            (unsigned char)std::min(255.0, finalColor.z()),
            255
        };
    }
} // namespace RayTracer
