#ifndef IRAYFEATURE_HPP_
#define IRAYFEATURE_HPP_
#include "integrators/Integrator.hpp"

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class IRayFeature {
        public:
            virtual ~IRayFeature() = default;
            virtual void process(RenderData &data, SurfaceIntersection &interaction) = 0;
    };
}

#endif