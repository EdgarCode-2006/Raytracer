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

    static void initializeIntersection(
        SurfaceIntersection &intersection,
        Graphic::Material *mat,
        const Math::Vector3D &hitPoint,
        const Triangle &triangle,
        const Math::Vector3D &normal,
        const Math::Vector3D &rayDir,
        std::pair<double, double> uvCoord
    )
    {
        intersection.mat = mat;
        intersection.hitPoint = hitPoint;
        intersection.tri = triangle;
        intersection.normal = normal;
        intersection.rayDir = rayDir;
        intersection.localColor = {0, 0, 0, 255};
        intersection.uv = uvCoord;
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

    void Integrator::executePipeline(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &intersection, ShaderType type)
    {
        for (auto &shaderIndex : this->_pipelineManager->_shaderMap[type]) {
            auto &shader = this->_pipelineManager->_pipeline[shaderIndex]; 
            if (shader->isActive())
                shader->process(scene, intersection);
        }
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
                tri,
                tri.normal,
                currentRay.direction,
                {tlasHit.u(), tlasHit.v()});
            intersection.throughput = currentRay.throughput;

            // run the UTILITY pipeline first
            this->executePipeline(scene, intersection, UTILITY);
            // run the material associated pipeline second.
            this->executePipeline(scene, intersection, intersection.mat->type);

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
            .hitPoint = {0, 0, 0},
            .rayDir = ray.direction,
            .tri = Triangle(),
            .localColor = {0, 0, 0, 255},
            .throughput = {1.0, 1.0, 1.0},
        };
        return consumeRays(ray, sceneLocked, intersection);
    }
} // namespace RayTracer
