#include "entities/Camera.hpp"
#include <cmath>
#include <numbers>

namespace RayTracer {
    Camera::Camera(const int width, const int height)
    {
        this->_fov = 100.0;
        this->_width = width;
        this->_height = height;
        this->_position = Math::Vector3D(0, 0, 0);
        this->_rotation = Math::Vector3D(0, 0, 0);
    }

    static Math::Vector3D rotateX(Math::Vector3D v, double deg)
    {
        double a = deg * std::numbers::pi / 180.0;
        return Math::Vector3D(v.x, v.y * cos(a) - v.z * sin(a), v.y * sin(a) + v.z * cos(a));
    }

    static Math::Vector3D rotateY(Math::Vector3D v, double deg)
    {
        double a = deg * std::numbers::pi / 180.0;
        return Math::Vector3D(v.x * cos(a) + v.z * sin(a), v.y, -v.x * sin(a) + v.z * cos(a));
    }

    void Camera::setPosition(const Math::Vector3D& position)
    {
        this->_position = position;
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

        Math::Vector3D center((minx+maxx)/2, (miny+maxy)/2, (minz+maxz)/2);
        double size = std::max(maxx-minx, std::max(maxy-miny, maxz-minz));

        this->_position = Math::Vector3D(center.x, center.y, center.z - size * 1.5);
    }

    Ray Camera::generateRay(int x, int y) const
    {
        double aspect = (double)this->_width / (double)this->_height;
        double scale  = std::tan(this->_fov / 2.0 * std::numbers::pi / 180.0);

        double u = (2.0 * (x + 0.5) / this->_width  - 1.0) * aspect * scale;
        double v = (2.0 * (y + 0.5) / this->_height - 1.0) * scale;

        Math::Vector3D dir = Math::Vector3D(u, -v, 1.0).normalize();
        dir = rotateX(dir, this->_rotation.x);
        dir = rotateY(dir, this->_rotation.y);

        return Ray(this->_position, dir);
    }
}
