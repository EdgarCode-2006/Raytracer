#include "Engine.hpp"

namespace RayTracer{
    Engine::Engine(RayTracer::Scene &scene) : _scene(scene)
    {
        Camera& cam = _scene.getCamera();
        _renderer = std::make_unique<Graphic::Renderer>(cam.getWidth(), cam.getHeight());
        _integrator = std::make_unique<Integrator>(_scene);
    }

    Engine::~Engine()
    {
        this->close();
    }

    void Engine::run()
    {
        Camera& cam = _scene.getCamera();

        while (!WindowShouldClose()) {
            _renderer->checkWindowState(cam);
            
            Color *pixels = _renderer->getPixels();
            _integrator->computePixels(cam, pixels);
            _renderer->renderPixelBuffer(pixels);
        }
    }

    void Engine::close()
    {
    }
}
