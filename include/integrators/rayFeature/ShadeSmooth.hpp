#ifndef SHADESMOOTH_HPP_
#define SHADESMOOTH_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class ShadeSmooth : public ARayFeature {
        public:
            ShadeSmooth() : ARayFeature("ShadeSmooth", UTILITY) {};
            void process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 1;};
    };
}
#endif