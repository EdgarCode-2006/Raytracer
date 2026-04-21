#include "Scene.hpp"

namespace RayTracer {
    Scene::Scene(const int width, const int height)
    {
        this->_camera = std::make_unique<Camera>(width, height);
    }
}