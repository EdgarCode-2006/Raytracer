#ifndef IRAYFEATURE_HPP_
#define IRAYFEATURE_HPP_
#include "integrators/Integrator.hpp"
#include "utils/helper.hpp"
#include <string>

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class IRayFeature {
        public:
            virtual ~IRayFeature() = default;
            virtual void process(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &interaction) = 0;
            virtual int getPriority() const noexcept = 0;
            virtual bool isActive() const noexcept = 0;
            virtual void setActivity(const bool &activity) noexcept = 0;
            virtual const std::string getName() const noexcept = 0;
    };
}

#endif