#ifndef SCENE_HPP_
#define SCENE_HPP_
#include <memory>
#include <vector>

#include "entities/Camera.hpp"
#include "utils/rayTracer.hpp"
#include "entities/Light.hpp"
#include "SceneObject.hpp"

namespace RayTracer {
    class Scene {
        public:
            Scene(const int width, const int height);
            ~Scene() = default;

            Camera& getCamera() { return *_camera; }

            void addObject(SceneObject obj) { _objects.push_back(std::move(obj)); }
            void addObject(std::vector<Triangle> triangles, Transform transform = {}) { _objects.emplace_back(std::move(triangles), transform); }

            std::vector<Triangle> getTriangles() const;

            std::vector<SceneObject>& getObjects() { return _objects; }
            const std::vector<SceneObject>& getObjects() const { return _objects; }

            void addLight(const DirectionalLight& light);
            void setAmbientLight(const AmbientLight& light);
            const std::vector<DirectionalLight>& getLights() const { return _directionalLights; }
            const AmbientLight& getAmbientLight() const { return _ambientLight; }
            void merge(const Scene& other);

        private:
            std::unique_ptr<Camera> _camera;
            std::vector<SceneObject> _objects;
            std::vector<DirectionalLight> _directionalLights;
            AmbientLight _ambientLight;
    };
}
#endif
