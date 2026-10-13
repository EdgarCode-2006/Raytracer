#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include "Scene.hpp"
#include "integrators/IntegrationManager.hpp"
#include "renderer/Renderer.hpp"
#include "input/InputManager.hpp"
#include "game/IGameState.hpp"
#include "game/GameAssets.hpp"

#include <map>
#include <memory>

namespace RayTracer {

    class Engine {
        public:
            Engine(RayTracer::Scene& scene, std::unique_ptr<Graphic::IRenderer> renderer, Game::GameAssets assets);
            ~Engine();

            void run();
            void generateSnapShot();
        private:
            void changeState(Game::StateId id);

            Scene& _scene;

            std::unique_ptr<Graphic::IRenderer> _renderer;
            std::unique_ptr<RayTracer::IntegrationManager> _integrationManager;
            std::shared_ptr<Frame> _currentFrame;

            Game::GameAssets _assets; // before the states, which keep references to it
            Input::InputManager _input;
            std::map<Game::StateId, std::unique_ptr<Game::IGameState>> _states;
            Game::StateId _currentStateId = Game::StateId::Menu;
            Game::IGameState *_currentState = nullptr;
    };

    class EngineException : public std::exception {
        public:
            EngineException(const std::string& message) : _message(message) {}
            const char* what() const noexcept override { return _message.c_str(); }
        private:
            std::string _message;
    };

}
#endif
