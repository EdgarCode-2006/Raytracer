#ifndef PLAYING_STATE_HPP_
#define PLAYING_STATE_HPP_

#include "game/IGameState.hpp"

namespace Game {

    // Placeholder for the shmup itself: for now it only goes back to the menu.
    class PlayingState : public IGameState {
        public:
            void onEnter() override { _next = StateId::Playing; }
            void update(const Input::InputManager& input) override;
            void drawOverlay() override;
            StateId next() const noexcept override { return _next; }

        private:
            StateId _next = StateId::Playing;
    };
}

#endif
