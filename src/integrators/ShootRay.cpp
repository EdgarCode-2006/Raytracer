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
        finalColor.x() += (localColor._r) * throughput.x();
        finalColor.y() += (localColor._g) * throughput.y();
        finalColor.z() += (localColor._b) * throughput.z();
    }

    // the scrolling background in front of the camera, the HDRI everywhere else (reflections)
    static Graphic::Pixel environmentColor(const RenderScene &scene, const Math::Vector3D &direction)
    {
        if (scene.background.covers(direction))
            return scene.background.sample(direction, scene.backgroundTime, scene.backgroundBrightness);
        return scene.lights.getHDRIPixel(direction * -1);
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
        intersection.tri = &triangle;
        intersection.normal = normal;
        intersection.rayDir = rayDir;
        intersection.localColor = {0, 0, 0, 255};
        intersection.tlasHit = tlasHit;
    }

    static void instanceNewRays(std::vector<Ray> &nextRays, std::vector<Ray> &rayQueue, const Ray &currentRayData = {})
    {
        for (auto &rayData : nextRays) {
            Ray newRay = {
                rayData.origin,
                rayData.direction,
                rayData.throughput,
                currentRayData.depth + 1,
            };
            rayQueue.push_back(newRay);
        }
        nextRays.clear();
    }

    void Integrator::executePipeline(const RenderScene &scene, SurfaceIntersection &intersection, ShaderType type)
    {
        for (IRayFeature *shader : this->_pipelineManager->getActiveFeatures(type))
            shader->process(scene, intersection);
    }

    Graphic::Pixel Integrator::shootRay(SurfaceIntersection &intersection, const RenderScene &scene, std::vector<Ray> &rayQueue)
    {
        // rayQueue is the FIFO of the rays left to trace, reused between calls
        size_t next = 0;

        rayQueue.clear();
        Math::Vector3D finalColor = {0.0, 0.0, 0.0};
        instanceNewRays(intersection.nextRays, rayQueue);

        while (next < rayQueue.size()) {
            Ray currentRay = rayQueue[next++];

            if (currentRay.depth >= MAX_DEPTH || currentRay.throughput.length() < MIN_THROUGHPUT_LENGTH)
                continue;

            const TLASHit tlasHit = scene.tlas->traverse(currentRay);
            if (!tlasHit.hasHit()) {
                const Graphic::Pixel envPixel = environmentColor(scene, currentRay.direction);
                integrateColor(finalColor, currentRay.throughput, envPixel);
                break;
            }
            const Instance &instance = scene.tlas->getInstance(tlasHit.instanceIndex());
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
            // the debug views switched on draw over every object, on top of its own shading
            if (intersection.mat->type != DEBUG)
                this->executePipeline(scene, intersection, DEBUG);

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
