#ifndef RAYTRACER_RAYHIT_HPP_
#define RAYTRACER_RAYHIT_HPP_

#include <cstdint>

namespace RayTracer {
    class RayHit {
        public:
            RayHit() = default;
            RayHit(double t, uint32_t primitiveIndex, double u, double v) noexcept :
            _t(t), _primitiveIndex(primitiveIndex), _u(u), _v(v) {}

            double t() const noexcept {return _t;}
            uint32_t primitiveIndex() const noexcept {return _primitiveIndex;}
            bool hasHit() const noexcept {return _t < 1e30;}
            double _u = 0.0;
            double _v = 0.0;

        private:
            double _t = 1e30;
            uint32_t _primitiveIndex = UINT32_MAX;
    };
}

#endif