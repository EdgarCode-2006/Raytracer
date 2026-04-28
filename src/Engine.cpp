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
        _integrator = std::make_unique<Integrator>(_scene);
        this->_integrator->instanceTiles(cam);
    }

    Engine::~Engine()
    {
        if (_integrator) {
            _integrator->waitForRendering();
        }
    }

    void Engine::handleWindowResize(RayTracer::Camera &cam)
    {
        int currentWidth = GetScreenWidth();
        int currentHeight = GetScreenHeight();
        
        if (currentWidth != _lastWidth || currentHeight != _lastHeight) {
            this->_integrator->waitForRendering();

            cam.setWidth(currentWidth);
            cam.setHeight(currentHeight);

            this->_renderer->resizeWindow(currentWidth, currentHeight);
            this->_integrator->instanceTiles(cam);

            _lastWidth = currentWidth;
            _lastHeight = currentHeight;
        }
    }

    void Engine::run()
    {
        Camera& cam = _scene.getCamera();
        const float sensitivity = 2.0f;
        const double speed = 0.05;

        while (this->_renderer->windowOpen()) {
            this->handleWindowResize(cam);

            if (IsKeyDown(KEY_LEFT)) cam.addRotationY(-sensitivity);
            if (IsKeyDown(KEY_RIGHT)) cam.addRotationY(sensitivity);
            if (IsKeyDown(KEY_UP)) cam.addRotationX(-sensitivity);
            if (IsKeyDown(KEY_DOWN)) cam.addRotationX(sensitivity);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse = GetMousePosition();
                Ray ray = cam.generateRay((int)mouse.x, (int)mouse.y);
                TLASHit hit = _scene.getTLAS().traverse(ray);
                if (hit.hasHit())
                    _scene.setSelectedInstance((int32_t)hit.instanceIndex());
                else
                    _scene.setSelectedInstance(-1);
            }

            if (IsKeyDown(KEY_W)) cam.moveForward(speed);
            if (IsKeyDown(KEY_S)) cam.moveForward(-speed);
            if (IsKeyDown(KEY_D)) cam.moveRight(speed);
            if (IsKeyDown(KEY_A)) cam.moveRight(-speed);
            if (IsKeyDown(KEY_E)) cam.moveUp(speed);
            if (IsKeyDown(KEY_Q)) cam.moveUp(-speed);

            Graphic::Pixel *pixels = _renderer->getPixels();
            if (!pixels)
                throw EngineException("Pixels buffer is empty!");
            _integrator->renderScene(cam, pixels);
            _renderer->renderPixelBuffer(pixels, _scene, cam);
        }
        _integrator->stop();
        _integrator->waitForRendering();
    }
}
