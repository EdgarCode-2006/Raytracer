#ifndef AABB_HPP_
#define AABB_HPP_

#include "utils/math.hpp"
#include <algorithm>

namespace Math {
    class AABB {

        public:
            AABB() noexcept;

            // expand(), min() and max() are defined here so they inline into the
            // BVH build and traversal loops.
            void expand(const Math::Vector3D &p) noexcept {
                _min[0] = std::min(_min[0], p[0]);
                _min[1] = std::min(_min[1], p[1]);
                _min[2] = std::min(_min[2], p[2]);

                _max[0] = std::max(_max[0], p[0]);
                _max[1] = std::max(_max[1], p[1]);
                _max[2] = std::max(_max[2], p[2]);
            }

            void expand(const AABB &other) noexcept {
                _min[0] = std::min(_min[0], other._min[0]);
                _min[1] = std::min(_min[1], other._min[1]);
                _min[2] = std::min(_min[2], other._min[2]);

                _max[0] = std::max(_max[0], other._max[0]);
                _max[1] = std::max(_max[1], other._max[1]);
                _max[2] = std::max(_max[2], other._max[2]);
            }

            bool isValid() const noexcept;
            double surfaceArea() const noexcept;
            Math::Vector3D center() const noexcept;
            Math::Vector3D size() const noexcept;
            // int longestAxis() const noexcept;

            const Math::Vector3D &min() const noexcept { return _min; }
            const Math::Vector3D &max() const noexcept { return _max; }

        private:
            Math::Vector3D _min;
            Math::Vector3D _max;

    };
}

#endif