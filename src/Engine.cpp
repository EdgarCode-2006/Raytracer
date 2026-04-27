#include "Engine.hpp"
#include "Scene.hpp"
#include <iostream>
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

        while (this->_renderer->windowOpen()) {
            this->handleWindowResize(cam);
            if (IsKeyDown(KEY_A)) {
                RayTracer::Matrix4 t;
                t = t.translation(1, 0, 0);

                auto &inst = _scene.getInstance(0);
                inst.setTransform(t * inst.getTransform());

                _scene.buildTLAS();
            }
            if (IsKeyDown(KEY_LEFT))
                cam.addRotationY(-2);
            if (IsKeyDown(KEY_RIGHT))
                cam.addRotationY( 2);
            if (IsKeyDown(KEY_UP))
                cam.addRotationX(-2);
            if (IsKeyDown(KEY_DOWN))
                cam.addRotationX( 2);

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
