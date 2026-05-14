#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include "Scene.hpp"
#include "integrators/Integrator.hpp"
#include "integrators/IntegrationManager.hpp"
#include "utils/Pixel.hpp"
#include "renderer/Renderer.hpp"

#include <vector>
#include <memory>

namespace RayTracer {

    class Engine {
        public:
            Engine(RayTracer::Scene& scene, std::unique_ptr<Graphic::IRenderer> renderer);
            ~Engine();
            
            void run();
            void generateSnapShot();
        private:
            int _lastWidth;
            int _lastHeight;

            Scene& _scene;
            void handleWindowResize(RayTracer::Camera &cam);
            bool moveCam(Camera &cam, Scene &scene);
            void handleFileDrop(Scene &scene);
            
            std::unique_ptr<Graphic::IRenderer> _renderer;
            std::unique_ptr<RayTracer::IntegrationManager> _integrationManager;
            std::shared_ptr<Frame> _currentFrame;

            bool _sceneDirty = false;
            int _frameCounter = 0;
            const int _snapshotInterval = 10;
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
