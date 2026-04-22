#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include <vector>

namespace RayTracer {

    class Camera {
        public:
            Camera(const int width, const int height);
            ~Camera() = default;
            Ray generateRay(int x, int y) const;

            int getHeight() const { return _height; }
            int getWidth()  const { return _width;  }

            void setHeight(int height) { _height = height; }
            void setWidth(int width)   { _width  = width;  }
            void setFov(double fov)    { _fov    = fov;    }

            const Math::Vector3D& getPosition() const { return _position; }
            double getFov() const { return _fov; }

            void setPosition(const Math::Vector3D& position);
            void fitToScene(const std::vector<Triangle>& triangles);

            void addRotationX(double degrees);
            void addRotationY(double degrees);

        private:
            void _updatePosition();

            Math::Vector3D _position;
            Math::Vector3D _target;
            double _distance = 1.0;
            double _yaw   = 0.0;
            double _pitch = 0.0;
            double _fov   = 60.0;
            int _width;
            int _height;
    };
}

#endif
