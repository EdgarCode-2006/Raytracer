#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "utils/math.hpp"
#include "utils/rayTracer.hpp"

namespace RayTracer {

    class Camera {
        public:
            Ray generateRay(int x, int y) const;
        private:
            Math::Vector3D _position;
            Math::Vector3D _rotation;
            double _fov;
            int _width;
            int _height;
    };
}

#endif
