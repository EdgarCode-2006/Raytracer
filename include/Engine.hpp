#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "integrators/Integrator.hpp"

#include <vector>
#include <memory>

namespace RayTracer {

    class Engine {
        public:
            Engine(RayTracer::Scene& scene);
            ~Engine();
            
            
            void run();
            void close();
        private:
            int _lastWidth;
            int _lastHeight;

            Scene& _scene;
            void handleWindowResize(RayTracer::Camera &cam);
            
            std::unique_ptr<Graphic::Renderer> _renderer;
            std::unique_ptr<RayTracer::Integrator> _integrator;
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
