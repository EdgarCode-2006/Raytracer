#ifndef TLASHIT_HPP_
#define TLASHIT_HPP_

#include <cstdint>

namespace RayTracer {
    class TLASHit {
        public:
            TLASHit() = default;
            TLASHit(double t, uint32_t primitiveIndex, uint32_t instanceIndex) noexcept
                : _t(t), _primitiveIndex(primitiveIndex), _instanceIndex(instanceIndex) {}

            double t() const noexcept {return _t;}
            uint32_t primitiveIndex() const noexcept {return _primitiveIndex;}
            uint32_t instanceIndex() const noexcept {return _instanceIndex;}
            bool hasHit() const noexcept {return _t < 1e30;}
        private:
            double _t = 1e30;
            uint32_t _primitiveIndex = UINT32_MAX;
            uint32_t _instanceIndex = UINT32_MAX;
    };
}



#endif