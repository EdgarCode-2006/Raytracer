#ifndef AABB_HPP_
#define AABB_HPP_

#include "utils/math.hpp"
namespace RayTracer {
    class AABB {
        
        public:
            AABB() noexcept;

            void expand(const Math::Vector3D &point) noexcept;
            void expand(const AABB &other) noexcept;

            bool isValid() const noexcept;
            double surfaceArea() const noexcept;
            Math::Vector3D center() const noexcept;
            Math::Vector3D size() const noexcept;
            // int longestAxis() const noexcept;

            // const Math::Vector3D &min() const noexcept;
            // const Math::Vector3D &max() const noexcept;
        
        private:
            Math::Vector3D _min;
            Math::Vector3D _max;

    };
}

#endif