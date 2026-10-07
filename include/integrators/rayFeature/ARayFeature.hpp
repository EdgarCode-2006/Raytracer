#ifndef ARAYFEATURE_HPP_
#define ARAYFEATURE_HPP_
#include "integrators/rayFeature/IRayFeature.hpp"

namespace RayTracer {
    struct RenderData;
    struct SurfaceIntersection;

    class ARayFeature : public IRayFeature {
        public:
            virtual ~ARayFeature() = default;
            ARayFeature(const std::string &name, const ShaderType &type) : _name(name), _type(type) {};
            virtual void process(const RenderScene &scene, SurfaceIntersection &interaction) override = 0;
            virtual int getPriority() const noexcept override = 0;
            virtual bool isActive() const noexcept override {return _active;};
            virtual const std::string getName() const noexcept override {return _name;};
            virtual ShaderType getType() const noexcept override {return _type;};

            virtual void setActivity(const bool &activity) noexcept override {this->_active = activity;};
        private:
            bool _active = true;
            std::string _name;
            ShaderType _type = DIFFUSION; // default to diffusion.
    };
}

#endif