#include "Scene.hpp"
#include "entities/Light.hpp"

namespace RayTracer {
    Scene::Scene(const int width, const int height)
    {
        this->_camera = std::make_unique<Camera>(width, height);
    }
   
    void Scene::addLight(const DirectionalLight& light)
    {
        this->_directionalLights.push_back(light);
    }
   
    void Scene::setAmbientLight(const AmbientLight& light)
    {
        this->_ambientLight = light;
    }

    std::vector<Triangle> Scene::getTriangles() const
    {
        std::vector<Triangle> result;
        for (const auto& obj : _objects)
            for (const auto& tri : obj.worldTriangles())
                result.push_back(tri);
        return result;
    }

    void Scene::merge(const Scene& other)
    {
        for (const auto& obj : other.getObjects())
            _objects.push_back(obj);
        for (const auto& light : other.getLights())
            _directionalLights.push_back(light);
    }

}
