#ifndef INTEGRATOR_HPP_
#define INTEGRATOR_HPP_

namespace RayTracer {
    class Integrator {
        public:
            Integrator();
            ~Integrator();
            void computePixels(const int &screenWidth, const int &screenHeight, Color *pixels);

        private:
    };
}
#endif
