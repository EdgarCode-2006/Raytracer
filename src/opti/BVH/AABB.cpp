#include "opti/BVH/AABB.hpp"
#include "utils/math.hpp"
#include <limits>

namespace Math {
    AABB::AABB() noexcept : _min(
        std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::infinity()
    ), _max(
        -std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity()
    ) {}

    bool AABB::isValid() const noexcept {
        return _min.x() <= _max.x() && _min.y() <= _max.y() && _min.z() <= _max.z();
    }

    Vector3D AABB::center() const noexcept {
        return (_min + _max) * 0.5;
    }

    Vector3D AABB::size() const noexcept {
        return _max - _min;
    }

    double AABB::surfaceArea() const noexcept {
        if (!isValid())
            return 0.0;
        double dx = _max.x() - _min.x();
        double dy = _max.y() - _min.y();
        double dz = _max.z() - _min.z();
        return 2.0 * (dx * dy + dy * dz + dx * dz);
    }
}