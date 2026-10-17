#ifndef INPUT_MANAGER_HPP_
#define INPUT_MANAGER_HPP_

#include "common/InputState.hpp"
#include <array>
#include <cstddef>
#include <string>
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
            static constexpr std::size_t MAX_KEYS_PER_ACTION = 2;

            InputManager();

            void update();

            bool isDown(Action action) const noexcept { return _down[index(action)]; }
            bool isPressed(Action action) const noexcept { return _pressed[index(action)]; }
            bool isReleased(Action action) const noexcept { return _released[index(action)]; }

            // the game buttons held now, to send to the server
            Common::InputState toInputState(uint32_t sequence) const noexcept;

            // the keys of an action, MAX_KEYS_PER_ACTION at most
            const std::vector<int> &getKeys(Action action) const noexcept { return _bindings[index(action)]; }
            // puts key in a slot of action, taking it from any other action using it
            void setKey(Action action, std::size_t slot, int key);
            void clearKey(Action action, std::size_t slot);
            void resetKeys();

            // the first key pressed during the last update, 0 if none: to rebind a key
            int getKeyPressed() const noexcept { return _keyPressed; }
            static std::string keyName(int key);

        private:
            static constexpr std::size_t ACTION_COUNT = static_cast<std::size_t>(Action::Count);
            static constexpr std::size_t index(Action action) noexcept { return static_cast<std::size_t>(action); }

            std::array<std::vector<int>, ACTION_COUNT> _bindings;
            std::array<bool, ACTION_COUNT> _down{};
            std::array<bool, ACTION_COUNT> _pressed{};
            std::array<bool, ACTION_COUNT> _released{};
            int _keyPressed = 0;
    };
}

#endif
