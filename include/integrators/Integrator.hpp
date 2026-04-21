#ifndef INTEGRATOR_HPP_
#define INTEGRATOR_HPP_
#include <memory>
#include "Scene.hpp"
#include <raylib.h>

namespace RayTracer {
    class Integrator {
        public:
            Integrator(const Scene& scene);
            ~Integrator();
            void computePixels(const Camera &cam, Color *pixels);

        private:
            const Scene& _scene;

    };
}
#endif
