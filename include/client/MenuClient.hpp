#ifndef MENU_CLIENT_HPP_
#define MENU_CLIENT_HPP_

#include "client/IClientMode.hpp"

namespace Client {

    // The menu opened with Esc during a game. It goes on top of the game, which stays connected
    // and visible (dimmed) under it.
    class MenuClient : public AClientMode {
        public:
            void update(const InputManager &input, double dt) override;
            void drawOverlay() override;
    };
}

#endif
