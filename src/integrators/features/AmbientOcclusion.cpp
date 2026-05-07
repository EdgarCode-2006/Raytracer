#include "integrators/rayFeature/AmbientOcclusion.hpp"
#include <cmath>

namespace RayTracer {
    void AmbientOcclusion::process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction)
    {
        double occlusion = 0.0;
        const double maxDistance = 2.0;

        for (int i = 0; i < AO_RAY_SAMPLE_COUNT; i++) {
            Math::Vector3D rayDir = interaction.getOmega();
            Ray AORay(interaction.hitPoint + (interaction.normal * 0.001), rayDir);

            const TLASHit tlasHit = scene->tlas->traverse(AORay);
            
            if (tlasHit.hasHit() && tlasHit.t() < maxDistance) {
                occlusion += (1.0 - (tlasHit.t() / maxDistance));
            }
        }
        double accessibility = 1.0 - (occlusion / AO_RAY_SAMPLE_COUNT);
        accessibility = std::clamp(accessibility, 0.0, 1.0);

        interaction.throughput.x() *= accessibility;
        interaction.throughput.y() *= accessibility;
        interaction.throughput.z() *= accessibility;
    }
}

extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::AmbientOcclusion();
}