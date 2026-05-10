#ifndef UV_HPP_
#define UV_HPP_

namespace Graphic
{
    class UV {
        public:
            double u, v;

            constexpr UV() : u(0.0), v(0.0) {}
            constexpr UV(double u, double v) : u(u), v(v) {}

            constexpr UV operator+(const UV& other) const {
                return { u + other.u, v + other.v };
            }

            constexpr UV operator*(double scalar) const {
                return { u * scalar, v * scalar };
            }

            friend UV operator*(double scalar, const UV& uv) {
                return { uv.u * scalar, uv.v * scalar };
            }

            UV& operator+=(const UV& other) {
                u += other.u;
                v += other.v;
                return *this;
            }
    };
} // namespace Graphic

#endif