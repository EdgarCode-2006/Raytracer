#include "integrators/rayFeature/BasicLighting.hpp"

namespace RayTracer {
    static void mapUV(SurfaceIntersection &interaction, double totalIntensity)
    {
        if (!interaction.mat || !interaction.mat->texture) return;
        auto [u, v] = interaction.tlasHit.uv;
        double w = 1 - u - v;

        auto [texU, texV] = (w * interaction.tri.uv[0]) +
                            (v * interaction.tri.uv[1]) +
                            (u * interaction.tri.uv[2]);
        interaction.localColor = interaction.mat->texture->sampleUVColor(texU, texV);
        interaction.localColor._r *= totalIntensity;
        interaction.localColor._g *= totalIntensity;
        interaction.localColor._b *= totalIntensity;
    }

    void BasicLighting::process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction)
    {
        double totalIntensity = scene->lights.getAmbientLight().intensity;

        for (const auto& light : scene->lights.getLights()) {
            Math::Vector3D lightDir = light.direction * -1;
            double diff = interaction.normal.dot(lightDir);

            if (diff > 0) {
                Ray shadowRay(interaction.hitPoint + (interaction.tri.normal * 0.001), lightDir);

                if (!scene->tlas->occluded(shadowRay))
                    totalIntensity += diff * light.intensity;
            }
        }

        for (const auto& light : scene->lights.getPointLights()) {
            Math::Vector3D toLight = light.position - interaction.hitPoint;
            double dist = toLight.length();
            if (dist < 1e-6) continue;
            Math::Vector3D lightDir = toLight / dist;
            double diff = interaction.normal.dot(lightDir);

            if (diff > 0) {
                Ray shadowRay(interaction.hitPoint + (interaction.tri.normal * 0.001), lightDir);

                // only what is between the point and the light casts a shadow
                if (!scene->tlas->occluded(shadowRay, dist))
                    totalIntensity += diff * light.intensity / (1.0 + dist * dist * 0.1);
            }
        }

        totalIntensity = std::clamp(totalIntensity, 0.0, 1.0);
        double r = totalIntensity, g = totalIntensity, b = totalIntensity;
        if (interaction.mat) {
            if (interaction.mat->texture != nullptr && interaction.mat->useTexture) {
                mapUV(interaction, totalIntensity);
                return;
            }
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
