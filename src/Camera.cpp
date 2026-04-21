#include "Camera.hpp"
#include <cmath>
#include <numbers>

namespace RayTracer {

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

    Ray Camera::generateRay(int x, int y) const
    {
        double aspect = (int)this->_width / (int)this->_height;
        double scale  = std::tan(this->_fov / 2.0 * std::numbers::pi / 180.0);

        double u = (2.0 * (x + 0.5) / this->_width  - 1.0) * aspect * scale;
        double v = (2.0 * (y + 0.5) / this->_height - 1.0) * scale;

        Math::Vector3D dir = Math::Vector3D(u, -v, 1.0).normalize();
        dir = rotateX(dir, this->_rotation.x);
        dir = rotateY(dir, this->_rotation.y);

        return Ray(this->_position, dir);
    }
}
