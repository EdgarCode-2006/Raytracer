#include "integrators/rayFeature/ShadeSmooth.hpp"
#include <cmath>

namespace RayTracer {
    void ShadeSmooth::process(const RenderScene &scene, SurfaceIntersection &interaction)
    {
        auto [u, v] = interaction.tlasHit.uv;
        double w = 1 - u - v;

        interaction.normal = ((w * interaction.tri->vn[0]) +
                            (u * interaction.tri->vn[1]) +
                            (v * interaction.tri->vn[2]));
    }
}

extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::ShadeSmooth();
}