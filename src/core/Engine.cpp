#include "core/Engine.hpp"
#include "core/Scene.hpp"
#include "game/MenuState.hpp"
#include "game/PlayingState.hpp"
#include <algorithm>
#include <chrono>

namespace RayTracer{
    static constexpr double MAX_FRAME_TIME = 0.1;

    Engine::Engine(RayTracer::Scene &scene, std::unique_ptr<Graphic::IRenderer> renderer, Game::GameAssets assets)
        : _scene(scene), _renderer(std::move(renderer)), _assets(std::move(assets))
    {
        if (!_renderer)
            throw EngineException("Invalid rendering object");
        Camera& cam = _scene.getCamera();
        // the window has a fixed size, the camera renders exactly that many pixels
        cam.setWidth(_renderer->getWidth());
        cam.setHeight(_renderer->getHeight());
        std::shared_ptr<const RenderScene> currentSnapshot = this->_scene.generateSceneSnapShot();
        _integrationManager = std::make_unique<IntegrationManager>(currentSnapshot);
        this->_integrationManager->instanceTiles(cam);
        auto* integrator = _integrationManager->getIntegrator();
        for (auto& feature : integrator->getPipeline()) {
            const auto& saved = _scene.getSavedFeatures();
            auto it = saved.find(feature->getName());
            if (it != saved.end())
                feature->setActivity(it->second);
        }

        _states[Game::StateId::Menu] = std::make_unique<Game::MenuState>(_scene);
        _states[Game::StateId::Playing] = std::make_unique<Game::PlayingState>(_scene, _assets);
        changeState(Game::StateId::Menu);
    }

    Engine::~Engine()
    {
        if (_integrationManager) {
            _integrationManager->waitForRendering();
        }
    }

    void Engine::generateSnapShot() {
        if (_scene.hasPendingHdri()) {
            _integrationManager->waitForRendering();
            _scene.applyPendingHdri();
        }
        this->_scene.buildTLAS();
        std::shared_ptr<const RenderScene> currentSnapshot = this->_scene.generateSceneSnapShot();
        this->_integrationManager->addSnapShot(currentSnapshot);
    }

    void Engine::changeState(Game::StateId id)
    {
        if (_currentState)
            _currentState->onExit();
        _currentStateId = id;
        _currentState = _states.at(id).get();
        _currentState->onEnter();
    }

    void Engine::run()
    {
        Camera& cam = _scene.getCamera();
        _currentFrame = std::make_shared<Frame>(cam.getWidth() * cam.getHeight(), 0);

        auto lastTime = std::chrono::steady_clock::now();

        while (this->_renderer->windowOpen()) {
            const auto now = std::chrono::steady_clock::now();
            // a long stall (window dragged, breakpoint) must not teleport everything
            const double dt = std::min(std::chrono::duration<double>(now - lastTime).count(), MAX_FRAME_TIME);
            lastTime = now;

            _input.update();
            _currentState->update(_input, dt);

            // the background scrolls and the entities move: every frame shows a new scene
            this->generateSnapShot();

            _integrationManager->renderSnapShot(cam);
            std::shared_ptr<Frame> tempFrame = _integrationManager->getAvailableFrame();
            if (tempFrame)
                _currentFrame = tempFrame;
            _renderer->renderPixelBuffer(_currentFrame->_pixelBuffer.data(), [this]() {
                _currentState->drawOverlay();
            });

            Game::StateId next = _currentState->next();
            if (next == Game::StateId::Quit) {
                _currentState->onExit();
                break;
            }
            if (next != _currentStateId)
                changeState(next);
        }
        _integrationManager->stop();
        _integrationManager->waitForRendering();
    }
}
