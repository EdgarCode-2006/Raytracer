#ifndef COMMON_INPUT_STATE_HPP_
#define COMMON_INPUT_STATE_HPP_

#include "common/Vec2.hpp"
#include <cstdint>

namespace Common {

    enum class Button : uint8_t {
        Up = 1 << 0,
        Down = 1 << 1,
        Left = 1 << 2,
        Right = 1 << 3,
        Fire = 1 << 4,
    };

    // What a client sends to the server every frame: the game buttons held, not the keys (the
    // key bindings are a client setting). The whole state is sent each time, so a lost one
    // doesn't leave a button stuck.
    struct InputState {
        uint32_t sequence = 0; // grows with every input sent, the server acknowledges the last one
        uint8_t buttons = 0;

        bool isDown(Button button) const noexcept { return buttons & static_cast<uint8_t>(button); }
        void set(Button button, bool down) noexcept {
            if (down)
                buttons |= static_cast<uint8_t>(button);
            else
                buttons &= ~static_cast<uint8_t>(button);
        }

        // -1 to 1 on each axis, x to the right and y up, not normalized
        Vec2 direction() const noexcept {
            return {
                (isDown(Button::Right) ? 1.0f : 0.0f) - (isDown(Button::Left) ? 1.0f : 0.0f),
                (isDown(Button::Up) ? 1.0f : 0.0f) - (isDown(Button::Down) ? 1.0f : 0.0f),
            };
        }
    };
}

#endif
