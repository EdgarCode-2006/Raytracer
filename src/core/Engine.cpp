#include "core/Engine.hpp"
#include "core/Scene.hpp"
#include "game/MenuState.hpp"
#include "game/PlayingState.hpp"

namespace RayTracer{
    Engine::Engine(RayTracer::Scene &scene, std::unique_ptr<Graphic::IRenderer> renderer) : _scene(scene), _renderer(std::move(renderer))
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

        _states[Game::StateId::Menu] = std::make_unique<Game::MenuState>();
        _states[Game::StateId::Playing] = std::make_unique<Game::PlayingState>();
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
        _currentStateId = id;
        _currentState = _states.at(id).get();
        _currentState->onEnter();
    }

    void Engine::run()
    {
        Camera& cam = _scene.getCamera();
        _currentFrame = std::make_shared<Frame>(cam.getWidth() * cam.getHeight(), 0);

        while (this->_renderer->windowOpen()) {
            _input.update();
            _currentState->update(_input);

            if (_sceneDirty) {
                this->generateSnapShot();
                _sceneDirty = false;
            }

            _integrationManager->renderSnapShot(cam);
            std::shared_ptr<Frame> tempFrame = _integrationManager->getAvailableFrame();
            if (tempFrame)
                _currentFrame = tempFrame;
            _renderer->renderPixelBuffer(_currentFrame->_pixelBuffer.data(), [this]() {
                _currentState->drawOverlay();
            });

            Game::StateId next = _currentState->next();
            if (next == Game::StateId::Quit)
                break;
            if (next != _currentStateId)
                changeState(next);
        }
        _integrationManager->stop();
        _integrationManager->waitForRendering();
    }
}
