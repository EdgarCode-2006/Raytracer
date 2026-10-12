#ifndef MENU_STATE_HPP_
#define MENU_STATE_HPP_

#include "game/IGameState.hpp"

namespace Game {

    class MenuState : public IGameState {
        public:
            void onEnter() override { _next = StateId::Menu; }
            void update(const Input::InputManager& input) override;
            void drawOverlay() override;
            StateId next() const noexcept override { return _next; }

        private:
            StateId _next = StateId::Menu;
    };
}

#endif
