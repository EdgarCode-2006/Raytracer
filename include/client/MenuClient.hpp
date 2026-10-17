#ifndef MENU_CLIENT_HPP_
#define MENU_CLIENT_HPP_

#include "client/IClientMode.hpp"
#include "client/SettingsPanel.hpp"

namespace Client {

    // The menu opened with Esc during a game. It goes on top of the game, which stays connected
    // and visible (dimmed) under it.
    class MenuClient : public AClientMode {
        public:
            explicit MenuClient(SettingsPanel &settings) : _settings(settings) {}

            void onEnter() override { _inSettings = false; }
            void update(const InputManager &input, double dt) override;
            void drawOverlay() override;

        private:
            SettingsPanel &_settings;
            bool _inSettings = false;
    };
}

#endif
