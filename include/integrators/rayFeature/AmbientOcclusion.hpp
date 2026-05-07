#ifndef AO_HPP_
#define AO_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"
#define AO_RAY_SAMPLE_COUNT 64

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class AmbientOcclusion : public ARayFeature {
        public:
            AmbientOcclusion() : ARayFeature("AmbientOcclusion") {};
            void process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 20;};
    };
}
#endif