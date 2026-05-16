#ifndef ILIGHT_HPP_
#define ILIGHT_HPP_

#include "IEntities.hpp"

namespace RayTracer {
    class ILight : public IEntities {
    public:
        virtual ~ILight() = default;
        virtual double getIntensity() const noexcept = 0;
        virtual void setIntensity(double intensity) noexcept = 0;
    };
}
#endif
