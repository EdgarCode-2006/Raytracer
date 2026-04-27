#ifndef RAYTRACER_RAYHIT_HPP_
#define RAYTRACER_RAYHIT_HPP_

#include <cstdint>

namespace RayTracer {
    class RayHit {
        public:
            RayHit() = default;
            RayHit(double t, uint32_t primitiveIndex) noexcept : _t(t), _primitiveIndex(primitiveIndex) {}

            double t() const noexcept {return _t;}
            uint32_t primitiveIndex() const noexcept {return _primitiveIndex;}
            bool hasHit() const noexcept {return _t < 1e30;}

        private:
            double _t = 1e30;
            uint32_t _primitiveIndex = UINT32_MAX;
    };
}

#endif