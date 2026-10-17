#ifndef TITLE_CLIENT_HPP_
#define TITLE_CLIENT_HPP_

#include "client/IClientMode.hpp"
#include "client/SettingsPanel.hpp"
#include "core/Scene.hpp"

namespace Client {

    // The title screen, before connecting: only the scrolling background and the menu.
    class TitleClient : public AClientMode {
        public:
            TitleClient(RayTracer::Scene &scene, SettingsPanel &settings) : _scene(scene), _settings(settings) {}

            void onEnter() override { _inSettings = false; }
            void update(const InputManager &input, double dt) override;
            void drawOverlay() override;

        private:
            RayTracer::Scene &_scene;
            SettingsPanel &_settings;
            bool _inSettings = false;
    };
}

#endif
