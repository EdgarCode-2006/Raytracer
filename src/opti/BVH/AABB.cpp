#include "opti/BVH/AABB.hpp"
#include "utils/math.hpp"
#include <limits>

namespace RayTracer {
    AABB::AABB() noexcept : _min(
        std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::infinity()
    ), _max(
        -std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity()
    ) {}

    void AABB::expand(const Math::Vector3D &p) noexcept {
        _min[0] = std::min(_min[0], p[0]);
        _min[1] = std::min(_min[1], p[1]);
        _min[2] = std::min(_min[2], p[2]);

        _max[0] = std::max(_max[0], p[0]);
        _max[1] = std::max(_max[1], p[1]);
        _max[2] = std::max(_max[2], p[2]);
    }

    void AABB::expand(const AABB &other) noexcept {
        _min[0] = std::min(_min[0], other._min[0]);
        _min[1] = std::min(_min[1], other._min[1]);
        _min[2] = std::min(_min[2], other._min[2]);

        _max[0] = std::max(_max[0], other._max[0]);
        _max[1] = std::max(_max[1], other._max[1]);
        _max[2] = std::max(_max[2], other._max[2]);
    }

    bool AABB::isValid() const noexcept {
        return _min.x() <= _max.x() && _min.y() <= _max.y() && _min.z() <= _max.z();
    }

    Math::Vector3D AABB::center() const noexcept {
        return (_min + _max) * 0.5;
    }

    Math::Vector3D AABB::size() const noexcept {
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