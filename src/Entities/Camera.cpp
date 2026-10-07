#include "entities/Camera.hpp"
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

    void Camera::addRotationX(double degrees)
    {
        _pitch += degrees;
        if (_pitch >  89.0) _pitch =  89.0;
        if (_pitch < -89.0) _pitch = -89.0;
        updateBasis();
    }

    void Camera::addRotationY(double degrees)
    {
        _yaw += degrees;
        updateBasis();
    }

    void Camera::moveForward(double dist)
    {
        _position += getForward() * dist;
    }

    void Camera::moveRight(double dist)
    {
        Math::Vector3D forward = getForward();
        Math::Vector3D right = Math::Vector3D(0, 1, 0).cross(forward).normalize();
        _position += right * dist;
    }

    void Camera::moveUp(double dist)
    {
        _position += Math::Vector3D(0, 1, 0) * dist;
    }

    void Camera::fitToScene(const std::vector<Triangle>& triangles)
    {
        double minx=1e9, maxx=-1e9, miny=1e9, maxy=-1e9, minz=1e9, maxz=-1e9;

        for (const auto& tri : triangles) {
            for (const auto* v : {&tri.a(), &tri.b(), &tri.c()}) {
                minx = std::min(minx, v->x()); maxx = std::max(maxx, v->x());
                miny = std::min(miny, v->y()); maxy = std::max(maxy, v->y());
                minz = std::min(minz, v->z()); maxz = std::max(maxz, v->z());
            }
        }

        Math::Vector3D center(
            (minx + maxx) / 2.0,
            (miny + maxy) / 2.0,
            (minz + maxz) / 2.0
        );
        double size = std::max(maxx - minx, std::max(maxy - miny, maxz - minz));

        _yaw   = 0.0;
        _pitch = 0.0;
        _position = Math::Vector3D(center.x(), center.y(), center.z() - size * 1.5);
        updateBasis();
    }

    Ray Camera::generateRay(int x, int y) const
    {
        double u = (2.0 * (x + 0.5) / _width  - 1.0) * _aspect * _scale;
        double v = (2.0 * (y + 0.5) / _height - 1.0) * _scale;

        Math::Vector3D dir = (_forward + _right * u - _up * v).normalize();
        return Ray(_position, dir);
    }
}
