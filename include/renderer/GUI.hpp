#ifndef GUI_HPP_
#define GUI_HPP_

#include "core/Scene.hpp"
#include "entities/Camera.hpp"

namespace Graphic {

    class GUI {
        public:
            void Render(RayTracer::Scene& scene, RayTracer::Camera& cam);
            bool wasSceneModified() const {return _sceneIsModified;};
        private:
            bool _sceneIsModified;
    };

}

#endif
