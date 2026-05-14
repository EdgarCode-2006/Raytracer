#include "integrators/rayFeature/debug/NormalMap.hpp"
#include <cmath>

namespace RayTracer {
    void NormalMap::process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction)
    {
        auto [u, v] = interaction.tlasHit.uv;
        double w = 1.0 - u - v;

        Math::Vector3D n = ((w * interaction.tri.vn[0]) +
                            (u * interaction.tri.vn[1]) +
                            (v * interaction.tri.vn[2])).normalize();

        interaction.localColor._r = static_cast<unsigned char>((n.x() + 1.0) * 127.5);
        interaction.localColor._g = static_cast<unsigned char>((n.y() + 1.0) * 127.5);
        interaction.localColor._b = static_cast<unsigned char>((n.z() + 1.0) * 127.5);

        interaction.normal = n;
    }
}

extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::NormalMap();
}