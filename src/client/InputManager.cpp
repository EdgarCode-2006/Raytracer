#include "client/InputManager.hpp"
#include <algorithm>
#include <raylib.h>

namespace Client {

    InputManager::InputManager()
    {
        resetKeys();
    }

    void InputManager::resetKeys()
    {
        _bindings[index(Action::MoveUp)] = {KEY_UP, KEY_W};
        _bindings[index(Action::MoveDown)] = {KEY_DOWN, KEY_S};
        _bindings[index(Action::MoveLeft)] = {KEY_LEFT, KEY_A};
        _bindings[index(Action::MoveRight)] = {KEY_RIGHT, KEY_D};
        _bindings[index(Action::Fire)] = {KEY_SPACE};
        _bindings[index(Action::Pause)] = {KEY_P};
        _bindings[index(Action::Confirm)] = {KEY_ENTER};
        _bindings[index(Action::Back)] = {KEY_ESCAPE};
    }

    void InputManager::setKey(Action action, std::size_t slot, int key)
    {
        if (slot >= MAX_KEYS_PER_ACTION)
            return;
        // a key does one thing only
        for (auto &keys : _bindings)
            std::erase(keys, key);

        auto &keys = _bindings[index(action)];
        if (slot < keys.size())
            keys[slot] = key;
        else
            keys.push_back(key);
    }

    void InputManager::clearKey(Action action, std::size_t slot)
    {
        auto &keys = _bindings[index(action)];
        if (slot < keys.size())
            keys.erase(keys.begin() + static_cast<std::ptrdiff_t>(slot));
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

        // empties raylib's queue of the keys pressed this frame, keeping the first
        _keyPressed = GetKeyPressed();
        while (GetKeyPressed() != 0) {}
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

    std::string InputManager::keyName(int key)
    {
        if ((key >= KEY_A && key <= KEY_Z) || (key >= KEY_ZERO && key <= KEY_NINE))
            return std::string(1, static_cast<char>(key));
        if (key >= KEY_F1 && key <= KEY_F12)
            return "F" + std::to_string(key - KEY_F1 + 1);
        if (key >= KEY_KP_0 && key <= KEY_KP_9)
            return "Keypad " + std::to_string(key - KEY_KP_0);
        switch (key) {
            case KEY_UP: return "Up";
            case KEY_DOWN: return "Down";
            case KEY_LEFT: return "Left";
            case KEY_RIGHT: return "Right";
            case KEY_SPACE: return "Space";
            case KEY_ENTER: return "Enter";
            case KEY_ESCAPE: return "Esc";
            case KEY_TAB: return "Tab";
            case KEY_BACKSPACE: return "Backspace";
            case KEY_LEFT_SHIFT: return "Left Shift";
            case KEY_RIGHT_SHIFT: return "Right Shift";
            case KEY_LEFT_CONTROL: return "Left Ctrl";
            case KEY_RIGHT_CONTROL: return "Right Ctrl";
            case KEY_LEFT_ALT: return "Left Alt";
            case KEY_RIGHT_ALT: return "Right Alt";
            case KEY_KP_ENTER: return "Keypad Enter";
            case KEY_COMMA: return ",";
            case KEY_PERIOD: return ".";
            case KEY_SEMICOLON: return ";";
            case KEY_SLASH: return "/";
            case KEY_MINUS: return "-";
            case KEY_EQUAL: return "=";
            default: return "Key " + std::to_string(key);
        }
    }
}
