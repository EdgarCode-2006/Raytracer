#ifndef OUTLINE_HPP_
#define OUTLINE_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"

#define ORTHOGONALITY_THRESHOLD 0.3

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class OutLine : public ARayFeature {
        public:
            OutLine() : ARayFeature("Outline", DEBUG) {};
            void process(const RenderScene &scene, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 101;};
    };
}
#endif