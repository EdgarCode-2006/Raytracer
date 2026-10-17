#include "client/InputManager.hpp"
#include <algorithm>
#include <raylib.h>

namespace Client {

    InputManager::InputManager()
    {
        bind(Action::MoveUp, KEY_UP);
        bind(Action::MoveUp, KEY_W);
        bind(Action::MoveDown, KEY_DOWN);
        bind(Action::MoveDown, KEY_S);
        bind(Action::MoveLeft, KEY_LEFT);
        bind(Action::MoveLeft, KEY_A);
        bind(Action::MoveRight, KEY_RIGHT);
        bind(Action::MoveRight, KEY_D);
        bind(Action::Fire, KEY_SPACE);
        bind(Action::Pause, KEY_P);
        bind(Action::Confirm, KEY_ENTER);
        bind(Action::Back, KEY_ESCAPE);
    }

    void InputManager::bind(Action action, int key)
    {
        auto& keys = _bindings[index(action)];
        if (std::find(keys.begin(), keys.end(), key) == keys.end())
            keys.push_back(key);
    }

    void InputManager::clearBindings(Action action)
    {
        _bindings[index(action)].clear();
    }

    void InputManager::update()
    {
        for (std::size_t i = 0; i < ACTION_COUNT; i++) {
            bool down = false;
            for (int key : _bindings[i])
                down = down || IsKeyDown(key);
            _pressed[i] = down && !_down[i];
            _released[i] = !down && _down[i];
            _down[i] = down;
        }
    }

    float InputManager::axisX() const noexcept
    {
        return (isDown(Action::MoveRight) ? 1.0f : 0.0f) - (isDown(Action::MoveLeft) ? 1.0f : 0.0f);
    }

    float InputManager::axisY() const noexcept
    {
        return (isDown(Action::MoveDown) ? 1.0f : 0.0f) - (isDown(Action::MoveUp) ? 1.0f : 0.0f);
    }

    Common::InputState InputManager::toInputState(uint32_t sequence) const noexcept
    {
        Common::InputState state;
        state.sequence = sequence;
        state.set(Common::Button::Up, isDown(Action::MoveUp));
        state.set(Common::Button::Down, isDown(Action::MoveDown));
        state.set(Common::Button::Left, isDown(Action::MoveLeft));
        state.set(Common::Button::Right, isDown(Action::MoveRight));
        state.set(Common::Button::Fire, isDown(Action::Fire));
        return state;
    }
}
