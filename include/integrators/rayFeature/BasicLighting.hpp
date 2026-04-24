#ifndef BASIC_LIGHT_HPP_
#define BASIC_LIGHT_HPP_
#include "integrators/rayFeature/IRayFeature.hpp"

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class BasicLighting : public IRayFeature {
        public:
            BasicLighting() = default;
            ~BasicLighting() override = default;
            void process(RenderData &data, SurfaceIntersection &interaction) override;
    };
}
#endif