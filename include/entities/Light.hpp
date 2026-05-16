#ifndef LIGHT_HPP_
#define LIGHT_HPP_

#include "ILight.hpp"
#include "utils/math.hpp"

namespace RayTracer {
    class AmbientLight : public ILight {
    public:
        AmbientLight() = default;
        double getIntensity() const noexcept override { return intensity; }
        void setIntensity(double i) noexcept override { intensity = i; }
        double intensity = 0.0;
    };

    class DirectionalLight : public ILight {
    public:
        DirectionalLight() = default;
        double getIntensity() const noexcept override { return intensity; }
        void setIntensity(double i) noexcept override { intensity = i; }
        Math::Vector3D direction;
        double intensity = 1.0;
    };

    class PointLight : public ILight {
    public:
        PointLight() = default;
        double getIntensity() const noexcept override { return intensity; }
        void setIntensity(double i) noexcept override { intensity = i; }
        Math::Vector3D position;
        double intensity = 1.0;
    };
}

#endif
