#ifndef LIGHT_HPP_
#define LIGHT_HPP_

#include "IEntities.hpp"
#include "utils/math.hpp"

namespace RayTracer {
    class AmbientLight : public IEntities {
        public:
            double intensity = 0.0;
    };

    class DirectionalLight : public IEntities {
        public:
            Math::Vector3D direction;
            double intensity = 1.0;
    };
}

#endif
