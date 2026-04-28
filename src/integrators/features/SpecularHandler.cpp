#include "integrators/rayFeature/SpecularHandler.hpp"

namespace RayTracer {
    void SpecularHandler::process(RenderData &data, SurfaceIntersection &interaction)
    {
        Math::Vector3D N = interaction.normal;
        float diff = interaction.rayDir.dot(interaction.normal);
        Math::Vector3D reflectionDir = (interaction.rayDir - N * (2.0f * diff)).normalize();

        double refraction = 0.67f;

        double Fo = (refraction - 1 / refraction + 1);
        Fo *= Fo; // reflectance at normal incidence.

        double reflectionValue = Fo + (1 - Fo) * pow((1 - (N.dot(interaction.rayDir))), 5);

        interaction.shouldBounce = true;
        
        interaction.nextRay = Ray(interaction.hitPoint + (N * 0.001f), reflectionDir);
        
        interaction.throughput.x() *= (1 / reflectionValue);
        interaction.throughput.y() *= (1 / reflectionValue);
        interaction.throughput.z() *= (1 / reflectionValue);
    }
}
extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::SpecularHandler();
}