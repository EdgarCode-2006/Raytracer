#ifndef BASIC_LIGHT_HPP_
#define BASIC_LIGHT_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"
#define AO_RAY_SAMPLE_COUNT 32

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class AmbientOcclusion : public ARayFeature {
        public:
            AmbientOcclusion() : ARayFeature("AmbientOcclusion") {};
            void process(RenderData &data, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 20;};
    };
}
#endif