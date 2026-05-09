#include "integrators/rayFeature/PathTracing.hpp"
#include <cmath>

namespace RayTracer {
    void PathTracing::process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction)
    {
        Math::Vector3D rayDir = interaction.getOmega(); 

        Math::Vector3D materialColor = {
            interaction.mat->color._r / 255.0,
            interaction.mat->color._g / 255.0,
            interaction.mat->color._b / 255.0
        };

        Math::Vector3D newThroughput = interaction.throughput * materialColor;

        Ray nextRay(interaction.hitPoint + (interaction.normal * 0.001), rayDir);
        nextRay.throughput = newThroughput;

        interaction.nextRays.push_back(nextRay);
        interaction.localColor = {0, 0, 0, 255};
    }
}

extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::PathTracing();
}