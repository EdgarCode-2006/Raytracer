#ifndef NORMALMAP_HPP_
#define NORMALMAP_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class NormalMap : public ARayFeature {
        public:
            NormalMap() : ARayFeature("NormalMap", DEBUG) {};
            void process(const RenderScene &scene, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 100;};
    };
}
#endif