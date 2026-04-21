#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "integrators/Integrator.hpp"

#include <vector>
#include <memory>

class Engine {
    public:
        Engine(const RayTracer::Scene& scene);
        ~Engine();

        void init();
        void run();
        void close();
    private:
        size_t _screenHeight = 0;
        size_t _screenWidth = 0;
        std::vector<RayTracer::Triangle> _triangles;

        std::unique_ptr<RayTracer::Scene>      _scene;
        std::unique_ptr<Graphic::Renderer>     _renderer;
        std::unique_ptr<RayTracer::Integrator> _integrator;
};

class EngineException : public std::exception {
    public:
        EngineException(const std::string& message) : _message(message) {}
        const char* what() const noexcept override { return _message.c_str(); }
    private:
        std::string _message;
};
#endif
