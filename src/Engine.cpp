#include "Engine.hpp"

namespace RayTracer{
    Engine::Engine(const RayTracer::Scene &scene)
    {
        this->init(scene);
    }

    Engine::~Engine()
    {
        this->close();
    }

    void Engine::init(const RayTracer::Scene &scene)
    {
        this->_renderer = std::make_unique<Graphic::Renderer>();
        this->_integrator = std::make_unique<Integrator>();
        // Copy the scene into a new scene, which holds the triangles.
    }

    void Engine::run()
    {
        // Color *pixels = this->_renderer.getPixels();
        // this->_integrator.computePixels(200, 800, pixels);
        // this->_renderer.renderPixelBuffer(pixels);
    }

    void Engine::close()
    {
    }
}
