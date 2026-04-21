#ifndef INTEGRATOR_HPP_
#define INTEGRATOR_HPP_
#include <memory>
#include "Scene.hpp"

namespace RayTracer {
    class Integrator {
        public:
            Integrator();
            ~Integrator();
            void computePixels(const int &screenWidth, const int &screenHeight, Color *pixels);

        private:
            std::unique_ptr<RayTracer::Scene> _scene;

    };
}
#endif
