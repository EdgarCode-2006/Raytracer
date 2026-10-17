#ifndef INPUT_MANAGER_HPP_
#define INPUT_MANAGER_HPP_

#include "common/InputState.hpp"
#include <array>
#include <cstddef>
#include <vector>

namespace Client {

    enum class Action : std::size_t {
        MoveUp,
        MoveDown,
        MoveLeft,
        MoveRight,
        Fire,
        Pause,
        Confirm,
        Back,
        Count
    };

    // Turns the keyboard into actions, read once per frame so every system sees the same input.
    // Only the client reads the keyboard: the server gets the game buttons (toInputState).
    class InputManager {
        public:
            InputManager();

            void update();
            void bind(Action action, int key);
            void clearBindings(Action action);

            bool isDown(Action action) const noexcept { return _down[index(action)]; }
            bool isPressed(Action action) const noexcept { return _pressed[index(action)]; }
            bool isReleased(Action action) const noexcept { return _released[index(action)]; }

            // -1 to 1 on each axis, x to the right and y down like the screen
            float axisX() const noexcept;
            float axisY() const noexcept;

            // the game buttons held now, to send to the server
            Common::InputState toInputState(uint32_t sequence) const noexcept;

        private:
            static constexpr std::size_t ACTION_COUNT = static_cast<std::size_t>(Action::Count);
            static constexpr std::size_t index(Action action) noexcept { return static_cast<std::size_t>(action); }

            std::array<std::vector<int>, ACTION_COUNT> _bindings;
            std::array<bool, ACTION_COUNT> _down{};
            std::array<bool, ACTION_COUNT> _pressed{};
            std::array<bool, ACTION_COUNT> _released{};
    };
}

#endif
