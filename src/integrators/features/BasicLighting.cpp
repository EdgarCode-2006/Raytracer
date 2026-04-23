#include "integrators/rayFeature/BasicLighting.hpp"

namespace RayTracer {
    void BasicLighting::process(RenderData &data, SurfaceIntersection &interaction)
    {
        double intensity = data.scene.getAmbientLight().intensity;
        for (const auto& light : data.scene.getLights()) {
            double diff = interaction.normal.dot(light.direction * -1.0);
            if (diff > 0)
                intensity += diff * light.intensity;
        }
        if (intensity > 1.0)
            intensity = 1.0;
        unsigned char c = (unsigned char)(intensity * 255);
        interaction.accumulatedColor = {c, c, c, 255};
    }

    extern "C" IRayFeature *getInstance() {
        return new BasicLighting();
    }
}