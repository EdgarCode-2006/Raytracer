#ifndef SCENE_HPP_
#define SCENE_HPP_
#include <memory>
#include <vector>

#include "entities/Camera.hpp"
#include "utils/rayTracer.hpp"

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

        private:
            std::unique_ptr<Camera> _camera;
            std::vector<Triangle> _triangles;
    };
}
#endif
