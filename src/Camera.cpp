#include "entities/Camera.hpp"
#include <cmath>
#include <numbers>

namespace RayTracer {

    Camera::Camera(const int width, const int height)
    {
        _fov      = 60.0;
        _width    = width;
        _height   = height;
        _position = Math::Vector3D(0, 0, 0);
        _target   = Math::Vector3D(0, 0, 0);
        _distance = 1.0;
        _yaw      = 0.0;
        _pitch    = 0.0;
    }

    void Camera::setPosition(const Math::Vector3D& position)
    {
        _position = position;
    }

    void Camera::addRotationX(double degrees)
    {
        _pitch += degrees;
        if (_pitch >  89.0) _pitch =  89.0;
        if (_pitch < -89.0) _pitch = -89.0;
        _updatePosition();
    }

    void Camera::addRotationY(double degrees)
    {
        _yaw += degrees;
        _updatePosition();
    }

    void Camera::_updatePosition()
    {
        double p = _pitch * std::numbers::pi / 180.0;
        double y = _yaw   * std::numbers::pi / 180.0;

        _position = Math::Vector3D(
            _target.x + _distance * std::sin(y) * std::cos(p),
            _target.y - _distance * std::sin(p),
            _target.z - _distance * std::cos(y) * std::cos(p)
        );
    }

    void Camera::fitToScene(const std::vector<Triangle>& triangles)
    {
        double minx=1e9, maxx=-1e9, miny=1e9, maxy=-1e9, minz=1e9, maxz=-1e9;

        for (const auto& tri : triangles) {
            for (const auto* v : {&tri.a, &tri.b, &tri.c}) {
                minx = std::min(minx, v->x); maxx = std::max(maxx, v->x);
                miny = std::min(miny, v->y); maxy = std::max(maxy, v->y);
                minz = std::min(minz, v->z); maxz = std::max(maxz, v->z);
            }
        }

        _target = Math::Vector3D(
            (minx + maxx) / 2.0,
            (miny + maxy) / 2.0,
            (minz + maxz) / 2.0
        );
        double size = std::max(maxx - minx, std::max(maxy - miny, maxz - minz));
        _distance = size * 1.5;
        _yaw   = 0.0;
        _pitch = 0.0;

        _updatePosition();
    }

    Ray Camera::generateRay(int x, int y) const
    {
        // Build orthonormal basis from position → target
        Math::Vector3D forward = (_target - _position).normalize();
        Math::Vector3D worldUp(0, 1, 0);
        Math::Vector3D right = worldUp.cross(forward).normalize();
        Math::Vector3D up    = forward.cross(right).normalize();

        double aspect = (double)_width  / (double)_height;
        double scale  = std::tan(_fov / 2.0 * std::numbers::pi / 180.0);

        double u = (2.0 * (x + 0.5) / _width  - 1.0) * aspect * scale;
        double v = (2.0 * (y + 0.5) / _height - 1.0) * scale;

        Math::Vector3D dir = (forward + right * u - up * v).normalize();
        return Ray(_position, dir);
    }
}
