#ifndef BASIC_LIGHT_HPP_
#define BASIC_LIGHT_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class SpecularHandler : public ARayFeature {
        public:
            SpecularHandler() : ARayFeature("SpecularHandler") {};
            void process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 0;};
    };
}
#endif