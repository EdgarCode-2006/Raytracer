#include "integrators/rayFeature/debug/OutLine.hpp"
#include <cmath>

namespace RayTracer {
    void OutLine::process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction)
    {
        double cosi = interaction.rayDir.dot(interaction.normal * -1);

        if (cosi <= 0.3 && cosi >= -0.3) {
            int value = static_cast<int>((1 - cosi) * 255);
            interaction.localColor._r = value;
            interaction.localColor._g = value;
            interaction.localColor._b = value;
        }
    }
}

extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::OutLine();
}