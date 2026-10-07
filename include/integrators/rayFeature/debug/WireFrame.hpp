#ifndef WIREFRAME_HPP_
#define WIREFRAME_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"
#define EDGE_LIMIT 0.01

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class WireFrame : public ARayFeature {
        public:
            WireFrame() : ARayFeature("WireFrame", DEBUG) {};
            void process(const RenderScene &scene, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 102;};
    };
}
#endif