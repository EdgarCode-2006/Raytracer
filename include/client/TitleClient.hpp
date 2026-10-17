#ifndef TITLE_CLIENT_HPP_
#define TITLE_CLIENT_HPP_

#include "client/IClientMode.hpp"
#include "core/Scene.hpp"

namespace Client {

    // The title screen, before connecting: only the scrolling background and the menu.
    class TitleClient : public AClientMode {
        public:
            explicit TitleClient(RayTracer::Scene &scene) : _scene(scene) {}

            void update(const InputManager &input, double dt) override;
            void drawOverlay() override;

        private:
            RayTracer::Scene &_scene;
    };
}

#endif
