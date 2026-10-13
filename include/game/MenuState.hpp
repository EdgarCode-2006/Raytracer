#ifndef MENU_STATE_HPP_
#define MENU_STATE_HPP_

#include "game/IGameState.hpp"
#include "core/Scene.hpp"

namespace Game {

    class MenuState : public IGameState {
        public:
            explicit MenuState(RayTracer::Scene& scene) : _scene(scene) {}

            void onEnter() override { _next = StateId::Menu; }
            void update(const Input::InputManager& input, double dt) override;
            void drawOverlay() override;
            StateId next() const noexcept override { return _next; }

        private:
            RayTracer::Scene& _scene;
            StateId _next = StateId::Menu;
    };
}

#endif
