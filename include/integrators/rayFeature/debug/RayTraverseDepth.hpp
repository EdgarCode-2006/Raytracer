#ifndef RAY_TRAVERSE_DEPTH_HPP_
#define RAY_TRAVERSE_DEPTH_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"
#define MAX_EXPECTED_DEPTH 25

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class RayTraverseDepth : public ARayFeature {
        public:
            RayTraverseDepth() : ARayFeature("TraverseDepth", DEBUG) {};
            void process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 104;};
    };
}
#endif