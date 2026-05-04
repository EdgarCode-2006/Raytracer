#include "integrators/rayFeature/BasicLighting.hpp"

namespace RayTracer {
    void BasicLighting::process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction)
    {
        double totalIntensity = scene->lights.getAmbientLight().intensity;

        for (const auto& light : scene->lights.getLights()) {
            Math::Vector3D lightDir = light.direction * -1;
            double diff = interaction.normal.dot(lightDir);
            
            if (diff > 0) {
                Ray shadowRay(interaction.hitPoint + (interaction.normal * 0.001), lightDir);
                bool blocked = false;

                const TLASHit tlasHit = scene->world.getTLAS().traverse(shadowRay);
                if (!tlasHit.hasHit())
                    totalIntensity += diff * light.intensity;
            }
        }
        totalIntensity = std::clamp(totalIntensity, 0.0, 1.0);
        double r = totalIntensity, g = totalIntensity, b = totalIntensity;
        if (interaction.mat) {
            r *= interaction.mat->color._r / 255.0;
            g *= interaction.mat->color._g / 255.0;
            b *= interaction.mat->color._b / 255.0;
        }
        interaction.localColor = {
            (unsigned char)(r * 255),
            (unsigned char)(g * 255),
            (unsigned char)(b * 255),
            255
        };
    }
}
extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::BasicLighting();
}
