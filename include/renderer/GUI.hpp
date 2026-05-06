#ifndef GUI_HPP_
#define GUI_HPP_

#include "core/Scene.hpp"
#include "entities/Camera.hpp"
#include <functional>

namespace RayTracer { class Integrator; }

namespace Graphic {

    class GUI {
        public:
            void Render(RayTracer::Scene& scene, RayTracer::Camera& cam);
            bool wasSceneModified() const {return _sceneIsModified;};
            void setIntegrator(RayTracer::Integrator* integrator) noexcept { _integrator = integrator; }
        private:
            bool _sceneIsModified = false;
            RayTracer::Integrator* _integrator = nullptr;
            char _savePath[256] = "scenes/saved.cfg";
            char _hdriPath[256] = "";
    };

}

#endif
