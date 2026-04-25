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

                for (const auto& tri : data.scene.getTriangles()) {
                    double t = RayTracer::Utils::intersectTriangle(shadowRay, tri);
                    if (t > 0.001) {
                        blocked = true;
                        break;
                    }
                }
                if (!blocked) {
                    totalIntensity += diff * light.intensity;
                }
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