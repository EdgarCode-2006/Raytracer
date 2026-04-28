#ifndef INSTANCE_HPP_
#define INSTANCE_HPP_

#include "opti/BVH/AABB.hpp"
#include "opti/BVH/BVH.hpp"
#include "opti/BVH/RayHit.hpp"
#include "utils/Matrix4.hpp"
#include "utils/Material.hpp"
#include "utils/rayTracer.hpp"
#include <cstdint>

namespace RayTracer {
    class Instance {
        public:
            Instance(const BVH *blas, const Triangle *triangles, uint32_t count, const Matrix4 &transform = Matrix4{}) noexcept : 
            _blas(blas), _triangles(triangles), _triCount(count) {
                setTransform(transform);
            }

            void setTransform(const Matrix4 &t) noexcept {
                _transform = t;
                _invTransform = t.inverse();
                _worldAABB = _transform.transformAABB(_blas->rootBounds());
            };
            
            RayHit intersect(const Ray &worldRay) const noexcept {
                return _blas->traverse(_invTransform.transformRay(worldRay));
            };
            
            const Matrix4& getTransform() const noexcept { return _transform; }
            const AABB &worldAABB() const noexcept { return _worldAABB; }
            const Triangle &getPrimitive(uint32_t idx) const noexcept { return _triangles[idx]; }

            Graphic::Material& getMaterial() noexcept { return _material; }
            const Graphic::Material& getMaterial() const noexcept { return _material; }
            void setMaterial(const Graphic::Material& mat) noexcept { _material = mat; }

        private:
            const BVH *_blas;
            const Triangle *_triangles;
            uint32_t _triCount;
            Matrix4 _transform;
            Matrix4 _invTransform;
            AABB _worldAABB;
            Graphic::Material _material;
    };
}


#endif