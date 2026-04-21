#include "Engine.hpp"

namespace RayTracer{
    Engine::Engine(RayTracer::Scene &scene) : _scene(scene)
    {
        Camera& cam = _scene.getCamera();
        _lastWidth = cam.getWidth();
        _lastHeight = cam.getHeight();
        _renderer = std::make_unique<Graphic::Renderer>(cam.getWidth(), cam.getHeight());
        _integrator = std::make_unique<Integrator>(_scene);
        this->_integrator->instanceTiles(cam);
    }

    Engine::~Engine()
    {
        this->close();
    }

    void Engine::handleWindowResize(RayTracer::Camera &cam)
    {
        int currentWidth = GetScreenWidth();
        int currentHeight = GetScreenHeight();
        
        if (currentWidth != _lastWidth || currentHeight != _lastHeight) {
            this->_integrator->waitForRendering();

            cam.setWidth(currentWidth);
            cam.setHeight(currentHeight);

            this->_renderer->resizeCanvas(currentWidth, currentHeight);
            this->_integrator->instanceTiles(cam);

            _lastWidth = currentWidth;
            _lastHeight = currentHeight;
        }
    }

    void Engine::run()
    {
        Camera& cam = _scene.getCamera();

        while (!WindowShouldClose()) {
            this->handleWindowResize(cam);
            
            Color *pixels = _renderer->getPixels();
            if (!pixels)
                throw EngineException("Pixels buffer is null!");
            _integrator->renderScene(cam, pixels);
            _renderer->renderPixelBuffer(pixels);
        }
    }

    void Engine::close()
    {
    }
}
