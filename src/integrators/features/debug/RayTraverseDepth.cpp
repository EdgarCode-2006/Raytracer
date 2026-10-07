#include "integrators/rayFeature/debug/RayTraverseDepth.hpp"

namespace RayTracer {
    void RayTraverseDepth::process(const RenderScene &scene, SurfaceIntersection &interaction)
    {
        float normalized = std::clamp((float)interaction.tlasHit.depthHit / MAX_EXPECTED_DEPTH, 0.0f, 1.0f);

        interaction.localColor._r = static_cast<unsigned char>(std::clamp(255.0f * (normalized * 2.0f - 1.0f), 0.0f, 255.0f));
        interaction.localColor._g = static_cast<unsigned char>(std::clamp(255.0f * (1.0f - std::abs(normalized * 2.0f - 1.0f)), 0.0f, 255.0f));
        interaction.localColor._b = static_cast<unsigned char>(std::clamp(255.0f * (1.0f - normalized * 2.0f), 0.0f, 255.0f));
    }
}

extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::RayTraverseDepth();
}