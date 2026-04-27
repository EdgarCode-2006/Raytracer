#include "integrators/rayFeature/BasicLighting.hpp"

namespace RayTracer {
    void BasicLighting::process(RenderData &data, SurfaceIntersection &interaction)
    {
        double totalIntensity = data.scene.getAmbientLight().intensity;

        for (const auto& light : data.scene.getLights()) {
            Math::Vector3D lightDir = light.direction * -1;
            double diff = interaction.normal.dot(lightDir);
            
            if (diff > 0) {
                Ray shadowRay(interaction.hitPoint + (interaction.normal * 0.001), lightDir);
                bool blocked = false;

                const TLASHit tlasHit = data.scene.getTLAS().traverse(shadowRay);
                if (!tlasHit.hasHit())
                    totalIntensity += diff * light.intensity;
            }
        }
        totalIntensity = std::clamp(totalIntensity, 0.0, 1.0);
        unsigned char c = (unsigned char)(totalIntensity * 255);
        interaction.localColor = {c, c, c, 255};
    }
}
extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::BasicLighting();
}