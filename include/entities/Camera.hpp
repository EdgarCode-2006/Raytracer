#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "utils/math.hpp"
#include "utils/rayTracer.hpp"

namespace RayTracer {

    class Camera {
        public:
            Camera(const int width, const int height);
            ~Camera() = default;
            Ray generateRay(int x, int y) const;

            const int getHeight() const {return this->_height;};
            const int getWidth() const {return this->_width;};

            void setHeight(const int height) {this->_height = height;};
            void setWidth(const int width) {this->_width = width;};
            void setFov(const double fov) {this->_fov = fov;};

            // void rotateCamera(double degrees)
            // void translateCamera(Vector3D newPos)
        private:
            Math::Vector3D _position;
            Math::Vector3D _rotation;
            double _fov = 1.0;
            int _width;
            int _height;
    };
}

#endif
