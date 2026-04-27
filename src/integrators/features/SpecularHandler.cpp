#include "integrators/rayFeature/SpecularHandler.hpp"

namespace RayTracer {
    void SpecularHandler::process(RenderData &data, SurfaceIntersection &interaction)
    {
        // look at material prop to tell wether the ray should reflect, refract, or diperse in every direction.
        Math::Vector3D N = interaction.normal;
        float diff = interaction.rayDir.dot(interaction.normal);
        if (diff > 0.0)
            N = N * -1;
        interaction.shouldBounce = true;
        Math::Vector3D reflectionDir = (interaction.rayDir - 2 * (interaction.rayDir.dot(N)) * N).normalize();
        reflectionDir *= 0.001; // slight bias to make reflection not perfect
        interaction.nextRay = Ray(interaction.hitPoint + (N * 0.0001), reflectionDir);
        interaction.throughput.x() *= 0.5;
        interaction.throughput.y() *= 0.5;
        interaction.throughput.z() *= 0.5;
    }
}
extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::SpecularHandler();
}