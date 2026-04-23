#ifndef SCENEOBJECT_HPP_
#define SCENEOBJECT_HPP_

#include "Transform.hpp"
#include "utils/rayTracer.hpp"
#include <vector>

namespace RayTracer {

class SceneObject {
    public:
        SceneObject(std::vector<Triangle> triangles, Transform transform) : _triangles(std::move(triangles)), _transform(transform) {}
    
        const std::vector<Triangle>& localTriangles() const { return _triangles; }
    
        Transform& getTransform() { return _transform; }
        const Transform& getTransform() const { return _transform; }
    
        std::vector<Triangle> worldTriangles() const
        {
            std::vector<Triangle> result;
            for (const auto& tri : _triangles)
                result.push_back(_transform.apply(tri));
            return result;
        }
    
    private:
        std::vector<Triangle> _triangles;
        Transform _transform;
    };
}

#endif
