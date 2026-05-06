#ifndef SCENEPARSER_HPP_
#define SCENEPARSER_HPP_

#include "core/Scene.hpp"
#include <string>
#include <vector>
#include <utility>

namespace RayTracer {

class SceneParser {
    public:
        void load(const std::string& path, Scene& scene);
        void save(const std::string& path, const Scene& scene,
            const std::vector<std::pair<std::string, bool>>& features = {});
};

}

#endif
