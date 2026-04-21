#ifndef RAYTRACER_HPP_
#define RAYTRACER_HPP_

#include "math.hpp"

namespace RayTracer {
        
    class Ray {
        public:
            Ray(Math::Vector3D origin, Math::Vector3D direction)
            {
                this->origin = origin;
                this->direction = direction;
            }
            Math::Vector3D origin;
            Math::Vector3D direction;
    };
    
    class Triangle {
        public:
            Triangle(Math::Vector3D a, Math::Vector3D b, Math::Vector3D c)
            {
                this->a = a;
                this->b = b;
                this->c = c;
            }
            Math::Vector3D a;
            Math::Vector3D b;
            Math::Vector3D c;
    };
}
#endif
