#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "integrators/Integrator.hpp"

#include <iostream>
#include <vector>
#include <memory>

/*
* This class implements the Engine of the raytracer, where all elements
* will be orchestrated, in order to produce an image.
*/
class Engine {
    // setup Scene from file

    // Do computation for each frame

    // Display frame.
    public:
        Engine(const Scene &scene);
        ~Engine();

        void init();
        void run();
        void close();
    private:
        // components
        size_t _screenHeight = 0;
        size_t _screenWidth = 0;
        std::vector<RayTracer::Triangle> _triangles; // going into BVH later

        // Logic
        std::unique_ptr<Scene> _scene;
        std::unique_ptr<Renderer> _renderer; // holds GUI
        std::unique_ptr<Integrator> _integrator;
};

class EngineException : public std::exception {
    public:
        EngineException(const std::string& message) : message_(message) {}
        const char* what() const throw() { return message_.c_str(); }

    private:
        std::string message_;
};
#endif