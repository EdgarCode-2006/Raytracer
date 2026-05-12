#include "integrators/rayFeature/debug/WireFrame.hpp"

namespace RayTracer {
    void WireFrame::process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction)
    {
        auto [u, v] = interaction.uv;
        double w = 1.0 - u - v;

        if (u <= EDGE_LIMIT || v <= EDGE_LIMIT || w <= EDGE_LIMIT) {
            interaction.localColor._r = 230;
            interaction.localColor._g = 230;
            interaction.localColor._b = 230;
        }
    }
}

extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::WireFrame();
}