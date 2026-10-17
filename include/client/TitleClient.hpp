#ifndef TITLE_CLIENT_HPP_
#define TITLE_CLIENT_HPP_

#include "client/IClientMode.hpp"
#include "core/Scene.hpp"

namespace Client {

    // The title screen, before connecting: only the scrolling background and the menu.
    class TitleClient : public IClientMode {
        public:
            explicit TitleClient(RayTracer::Scene &scene) : _scene(scene) {}

            void onEnter() override { _next = ModeId::Title; }
            void update(const InputManager &input, double dt) override;
            void drawOverlay() override;
            ModeId next() const noexcept override { return _next; }

        private:
            RayTracer::Scene &_scene;
            ModeId _next = ModeId::Title;
    };
}

#endif
