#ifndef SPECULAR_HPP_
#define SPECULAR_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class SpecularHandler : public ARayFeature {
        public:
            SpecularHandler() : ARayFeature("SpecularHandler", CAUSTIC) {};
            void process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 30;};
    };
}
#endif