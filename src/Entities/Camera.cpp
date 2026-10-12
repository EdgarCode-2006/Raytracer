#include "entities/Camera.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace RayTracer {

    Camera::Camera(const int width, const int height)
        : _position(0, 0, 0), _yaw(0.0), _pitch(0.0), _fov(60.0), _width(width), _height(height)
    {
        updateBasis();
    }

    void Camera::updateBasis()
    {
        double p = _pitch * std::numbers::pi / 180.0;
        double y = _yaw   * std::numbers::pi / 180.0;
        _forward = Math::Vector3D(
            std::sin(y) * std::cos(p),
            -std::sin(p),
            std::cos(y) * std::cos(p)
        ).normalize();
        _right = Math::Vector3D(0, 1, 0).cross(_forward).normalize();
        _up    = _forward.cross(_right).normalize();

        _aspect = (double)_width  / (double)_height;
        _scale  = std::tan(_fov / 2.0 * std::numbers::pi / 180.0);
    }

    void Camera::setOrientation(double yaw, double pitch)
    {
        _yaw   = yaw;
        _pitch = std::clamp(pitch, -89.0, 89.0);
        updateBasis();
    }

    Ray Camera::generateRay(double x, double y) const
    {
        double u = (2.0 * (x + 0.5) / _width  - 1.0) * _aspect * _scale;
        double v = (2.0 * (y + 0.5) / _height - 1.0) * _scale;

        Math::Vector3D dir = (_forward + _right * u - _up * v).normalize();
        return Ray(_position, dir);
    }
}
