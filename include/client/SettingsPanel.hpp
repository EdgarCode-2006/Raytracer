#ifndef SETTINGS_PANEL_HPP_
#define SETTINGS_PANEL_HPP_

#include "client/ClientSettings.hpp"
#include "client/InputManager.hpp"
#include "core/Engine.hpp"
#include <cstddef>
#include <optional>

namespace Client {

    // The client's settings, drawn inside the window of the title screen or of the menu.
    // They only change this client: the server never hears about keys or graphics.
    class SettingsPanel {
        public:
            SettingsPanel(InputManager &input, RayTracer::Engine &engine, ClientSettings &settings)
                : _input(input), _engine(engine), _settings(settings) {}

            void draw();
            // waiting for the key to bind: Esc cancels it, so it isn't "back" meanwhile
            bool isCapturingKey() const noexcept { return _capture.has_value(); }

        private:
            struct Capture {
                Action action;
                std::size_t slot;
            };

            void drawControls();
            void drawGraphics();
            void drawAccessibility();
            void captureKey();

            InputManager &_input;
            RayTracer::Engine &_engine;
            ClientSettings &_settings;
            std::optional<Capture> _capture;
            // the slot that just got its key: turning the navigation off to wait for the key
            // lost its focus, it gets it back so that the arrows go on from there
            std::optional<Capture> _refocus;
    };
}

#endif
