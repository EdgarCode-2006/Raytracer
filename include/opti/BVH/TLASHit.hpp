#ifndef TLASHIT_HPP_
#define TLASHIT_HPP_

#include <cstdint>

namespace RayTracer {
    class TLASHit {
        public:
            TLASHit() = default;
            TLASHit(double t, uint32_t primitiveIndex, uint32_t instanceIndex, double u, double v) noexcept
                : _t(t), _primitiveIndex(primitiveIndex), _instanceIndex(instanceIndex), _u(u), _v(v) {}

            double t() const noexcept {return _t;}
            uint32_t primitiveIndex() const noexcept {return _primitiveIndex;}
            uint32_t instanceIndex() const noexcept {return _instanceIndex;}
            bool hasHit() const noexcept {return _t < 1e30;}

            double u() const noexcept {return this->_u;};
            double v() const noexcept {return this->_v;};
            double w() const noexcept {return 1 - this->_u - this->_v;};

        private:
            double _t = 1e30;
            double _u = 0.0;
            double _v = 0.0;
            uint32_t _primitiveIndex = UINT32_MAX;
            uint32_t _instanceIndex = UINT32_MAX;
    };
}



#endif