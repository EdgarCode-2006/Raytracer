#include "integrators/rayFeature/SpecularHandler.hpp"

namespace RayTracer {
    void SpecularHandler::process(RenderData &data, SurfaceIntersection &interaction)
    {
        Math::Vector3D N = interaction.normal;
        float diff = interaction.rayDir.dot(interaction.normal);
        Math::Vector3D reflectionDir = (interaction.rayDir - N * (2.0f * diff)).normalize();

        interaction.shouldBounce = true;
        
        interaction.nextRay = Ray(interaction.hitPoint + (N * 0.001f), reflectionDir);
        
        interaction.throughput.x() *= 0.5f;
        interaction.throughput.y() *= 0.5f;
        interaction.throughput.z() *= 0.5f;
    }
}
extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::SpecularHandler();
}