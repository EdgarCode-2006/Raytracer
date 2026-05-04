#include "integrators/rayFeature/AmbientOcclusion.hpp"
#include <cmath>

namespace RayTracer {
    void AmbientOcclusion::process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction)
    {
        double hitNothing = 0;
        const double maxDistance = 1.0;

        // Create N rays in the hemisphere of the hit
        for (int i = 0; i < AO_RAY_SAMPLE_COUNT; i++) {
            Math::Vector3D rayDir = interaction.getOmega();
            Ray AORay(interaction.hitPoint + (interaction.normal * 0.001), rayDir);
            bool blocked = false;

            const TLASHit tlasHit = scene->tlas->traverse(AORay);
            if (!blocked)
                hitNothing += 1.0;
        }
        double intensity = hitNothing / AO_RAY_SAMPLE_COUNT;

        interaction.throughput.x() *= intensity;
        interaction.throughput.y() *= intensity;
        interaction.throughput.z() *= intensity;
    }
}

extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::AmbientOcclusion();
}