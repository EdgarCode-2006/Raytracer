#ifndef SCENE_HPP_
#define SCENE_HPP_

#include "entities/Camera.hpp"

namespace RayTracer {
    class Scene {
        public:
            Camera getCamera() {return _camera;};
        
        private:
            Camera _camera;

    };
}
#endif
