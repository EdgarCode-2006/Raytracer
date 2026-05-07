#include "integrators/rayFeature/Emission.hpp"

namespace RayTracer {
    void Emission::process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction)
    {
        if (!interaction.mat || interaction.mat->emission <= 0.0f)
            return;

        float emitStrength = interaction.mat->emission;
        
        interaction.localColor._r = std::min(255, interaction.localColor._r + (int)(interaction.mat->color._r * emitStrength));
        interaction.localColor._g = std::min(255, interaction.localColor._g + (int)(interaction.mat->color._g * emitStrength));
        interaction.localColor._b = std::min(255, interaction.localColor._b + (int)(interaction.mat->color._b * emitStrength));
    }
}

extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::Emission();
}