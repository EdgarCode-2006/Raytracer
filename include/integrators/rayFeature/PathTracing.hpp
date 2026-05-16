#ifndef PATHTRACING_HPP_
#define PATHTRACING_HPP_
#include "integrators/rayFeature/ARayFeature.hpp"
#define PATH_SAMPLE_COUNT 50

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class PathTracing : public ARayFeature {
        public:
            PathTracing() : ARayFeature("PathTracing", DIFFUSION) {};
            void process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction) override;
            int getPriority() const noexcept override {return 10;};
    };
}
#endif