#ifndef IGAME_STATE_HPP_
#define IGAME_STATE_HPP_

#include "input/InputManager.hpp"

namespace Game {

    enum class StateId {
        Menu,
        Playing,
        Quit
    };

    class IGameState {
        public:
            virtual ~IGameState() = default;

            virtual void onEnter() {}
            virtual void update(const Input::InputManager& input) = 0;
            // ImGui calls, made between rlImGuiBegin and rlImGuiEnd by the renderer
            virtual void drawOverlay() = 0;
            // the state to run on the next frame, itself to keep running
            virtual StateId next() const noexcept = 0;
    };
}

#endif
