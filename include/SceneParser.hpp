#ifndef SCENEPARSER_HPP_
#define SCENEPARSER_HPP_

#include "Scene.hpp"
#include <string>

namespace RayTracer {

class SceneParser {
    public:
        void load(const std::string& path, Scene& scene);
};

}

#endif
