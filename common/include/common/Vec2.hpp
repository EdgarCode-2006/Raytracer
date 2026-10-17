#ifndef COMMON_VEC2_HPP_
#define COMMON_VEC2_HPP_

#include <cmath>

namespace Common {

    // A position or a velocity on the plane the game is played on: x to the right, y up.
    struct Vec2 {
        float x = 0.0f;
        float y = 0.0f;

        constexpr Vec2 operator+(const Vec2 &o) const noexcept { return {x + o.x, y + o.y}; }
        constexpr Vec2 operator-(const Vec2 &o) const noexcept { return {x - o.x, y - o.y}; }
        constexpr Vec2 operator*(float s) const noexcept { return {x * s, y * s}; }
        constexpr Vec2 &operator+=(const Vec2 &o) noexcept { x += o.x; y += o.y; return *this; }
        constexpr bool operator==(const Vec2 &) const noexcept = default;

        float length() const noexcept { return std::sqrt(x * x + y * y); }
        Vec2 normalized() const noexcept {
            const float len = length();
            return len > 0.0f ? Vec2{x / len, y / len} : Vec2{};
        }
    };
}

#endif
