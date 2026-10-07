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
            // x and y are pixel coordinates: fractional parts give rays inside the pixel
            Ray generateRay(double x, double y) const;

            int getHeight() const { return _height; }
            int getWidth()  const { return _width;  }

            void setHeight(int height) { _height = height; updateBasis(); }
            void setWidth(int width)   { _width  = width;  updateBasis(); }
            void setFov(double fov)    { _fov    = fov;    updateBasis(); }

            const Math::Vector3D& getPosition() const { return _position; }
            double getFov() const { return _fov; }
            double getYaw() const { return _yaw; }
            double getPitch() const { return _pitch; }

            void setPosition(const Math::Vector3D& position) { _position = position; }
            void fitToScene(const std::vector<Triangle>& triangles);

            void addRotationX(double degrees);
            void addRotationY(double degrees);

            void moveForward(double dist);
            void moveRight(double dist);
            void moveUp(double dist);
            const Math::Vector3D &getForward() const { return _forward; }

        private:
            // Recomputes what generateRay needs, every time the orientation, fov or size change.
            void updateBasis();

            Math::Vector3D _position;
            double _yaw   = 0.0;
            double _pitch = 0.0;
            double _fov   = 60.0;
            int _width;
            int _height;

            Math::Vector3D _forward;
            Math::Vector3D _right;
            Math::Vector3D _up;
            double _aspect = 1.0;
            double _scale = 1.0;
    };
}

#endif
