#include "Engine.hpp"
#include "Scene.hpp"
#include "opti/BVH/TLASHit.hpp"
#include <raylib.h>

namespace RayTracer{
    Engine::Engine(RayTracer::Scene &scene, std::unique_ptr<Graphic::IRenderer> renderer) : _scene(scene), _renderer(std::move(renderer))
    {
        if (!_renderer)
            throw EngineException("Invalid rendering object");
        Camera& cam = _scene.getCamera();
        _lastWidth = cam.getWidth();
        _lastHeight = cam.getHeight();
        std::shared_ptr<const RenderScene> currentSnapshot = this->_scene.generateSceneSnapShot();
        _integrationManager = std::make_unique<IntegrationManager>(currentSnapshot);
        this->_integrationManager->instanceTiles(cam);
        this->_renderer->onSnapshotRequested = [this]() {
            this->_sceneDirty = true;
        };
    }

    Engine::~Engine()
    {
        if (_integrationManager) {
            _integrationManager->waitForRendering();
        }
    }

    void Engine::generateSnapShot() {
        std::shared_ptr<const RenderScene> currentSnapshot = this->_scene.generateSceneSnapShot();
        this->_integrationManager->addSnapShot(currentSnapshot);
    }

    void Engine::handleWindowResize(RayTracer::Camera &cam)
    {
        int currentWidth = GetScreenWidth();
        int currentHeight = GetScreenHeight();

        if (currentWidth != _lastWidth || currentHeight != _lastHeight) {
            this->_integrationManager->waitForRendering();

            cam.setWidth(currentWidth);
            cam.setHeight(currentHeight);

            this->_renderer->resizeWindow(currentWidth, currentHeight);
            this->_integrationManager->instanceTiles(cam);

            _lastWidth = currentWidth;
            _lastHeight = currentHeight;
        }
    }

    bool Engine::moveCam(Camera &cam, Scene &scene)
    {
        const float sensitivity = 2.0f;
        const double speed = 0.05;
        bool changed = false;

        if (IsKeyDown(KEY_LEFT)) { cam.addRotationY(-sensitivity); changed = true; }
        if (IsKeyDown(KEY_RIGHT)) { cam.addRotationY(sensitivity); changed = true; }
        if (IsKeyDown(KEY_UP)) { cam.addRotationX(-sensitivity); changed = true; }
        if (IsKeyDown(KEY_DOWN)) { cam.addRotationX(sensitivity); changed = true; }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            Ray ray = cam.generateRay((int)mouse.x, (int)mouse.y);
            TLASHit hit = scene.getTLAS().traverse(ray);
            if (hit.hasHit())
                scene.setSelectedInstance((int32_t)hit.instanceIndex());
            else
                scene.setSelectedInstance(-1);
            changed = true;
        }

        if (IsKeyDown(KEY_W)) { cam.moveForward(speed); changed = true; }
        if (IsKeyDown(KEY_S)) { cam.moveForward(-speed); changed = true; }
        if (IsKeyDown(KEY_D)) { cam.moveRight(speed); changed = true; }
        if (IsKeyDown(KEY_A)) { cam.moveRight(-speed); changed = true; }
        if (IsKeyDown(KEY_E)) { cam.moveUp(speed); changed = true; }
        if (IsKeyDown(KEY_Q)) { cam.moveUp(-speed); changed = true; }

        return changed;
    }

    void Engine::run()
    {
        Camera& cam = _scene.getCamera();
        std::shared_ptr<Frame> currentFrame = std::make_shared<Frame>(cam.getWidth() * cam.getHeight(), 0);

        while (this->_renderer->windowOpen()) {
            _frameCounter++;

            // GUI
            this->handleWindowResize(cam);
            this->moveCam(cam, this->_scene);

            if (_sceneDirty && (_frameCounter % _snapshotInterval == 0)) {
                this->generateSnapShot();
                _sceneDirty = false;
            }
            
            _integrationManager->renderSnapShot(cam);
            std::shared_ptr<Frame> tempFrame = _integrationManager->getAvailableFrame();
            if (tempFrame)
                currentFrame = tempFrame;
            _renderer->renderPixelBuffer(currentFrame->_pixelBuffer.data(), _scene, cam);
        }
        _integrationManager->stop();
        _integrationManager->waitForRendering();
    }
}
