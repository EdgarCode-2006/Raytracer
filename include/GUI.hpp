#ifndef GUI_HPP_
#define GUI_HPP_

#include "Scene.hpp"
#include "entities/Camera.hpp"

namespace Graphic {

    class GUI {
        public:
            void Render(RayTracer::Scene& scene, RayTracer::Camera& cam);
    };

}

#endif
