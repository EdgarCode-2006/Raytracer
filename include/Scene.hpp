#ifndef SCENE_HPP_
#define SCENE_HPP_
#include <memory>
#include <vector>

#include "entities/Camera.hpp"
#include "utils/rayTracer.hpp"
#include "entities/Light.hpp"

namespace RayTracer {
    class Scene {
        public:
            Scene(const int width, const int height);
            ~Scene() = default;
            
            Camera& getCamera() {return *_camera;};
            void addTriangles(const std::vector<Triangle>& triangles) {
                _triangles = triangles;
            }
            const std::vector<Triangle>& getTriangles() const { return _triangles; }
            void addLight(const DirectionalLight& light);
            void setAmbientLight(const AmbientLight& light);
            const std::vector<DirectionalLight>& getLights() const { return _directionalLights; }
            const AmbientLight& getAmbientLight() const { return _ambientLight; }
            void merge(const Scene& other);

        private:
            std::unique_ptr<Camera> _camera;
            std::vector<Triangle> _triangles;
            std::vector<DirectionalLight> _directionalLights;
            AmbientLight _ambientLight;
    };
}
#endif
