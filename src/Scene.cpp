#include "Scene.hpp"
#include "entities/Light.hpp"

namespace RayTracer {
    Scene::Scene(const int width, const int height) :
    _camera(width, height),
    _hdri(HDRI("assets/HDRIs/HDRI.exr"))
    {}
   
    void Scene::addLight(const DirectionalLight& light)
    {
        this->_directionalLights.push_back(light);
    }
   
    void Scene::setAmbientLight(const AmbientLight& light)
    {
        this->_ambientLight = light;
    }

    // void Scene::merge(const Scene &other)
    // {
    //      for (const auto& tri : other.getTriangles())
    //          this->_triangles.push_back(tri);
    //      for (const auto& light : other.getLights())
    //          this->_directionalLights.push_back(light);
    // }
}
