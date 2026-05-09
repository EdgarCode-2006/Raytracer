#ifndef EMISSION_HPP_
#define EMISSION_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class Emission : public ARayFeature {
        public:
            Emission() : ARayFeature("Emission", EMISSION) {};
            void process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 1;};
    };
}
#endif