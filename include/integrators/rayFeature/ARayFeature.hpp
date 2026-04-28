#ifndef ARAYFEATURE_HPP_
#define ARAYFEATURE_HPP_
#include "integrators/rayFeature/IRayFeature.hpp"

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class ARayFeature : public IRayFeature {
        public:
            virtual ~ARayFeature() = default;
            ARayFeature(const std::string &name) : _name(name) {};
            virtual void process(RenderData &data, SurfaceIntersection &interaction) override = 0;
            virtual int getPriority() const noexcept override = 0;
            virtual bool isActive() const noexcept override {return _active;};
            virtual const std::string getName() const noexcept {return _name;};

            virtual void setActivity(const bool &activity) noexcept override {this->_active = activity;};
        private:
            bool _active = true;
            std::string _name;
    };
}

#endif