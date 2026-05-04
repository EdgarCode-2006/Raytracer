#ifndef INSTANCE_HPP_
#define INSTANCE_HPP_

#include "core/SceneComponents/GeometricData.hpp"
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
            Instance(const GeometricData *geo, uint32_t meshId,
                const Math::Matrix4 &transform = Math::Matrix4{}) noexcept
            : _geometricData(geo), _meshId(meshId)
            {
                setTransform(transform);
            }

            void setTransform(const Math::Matrix4 &t) noexcept {
                _transform    = t;
                _invTransform = t.inverse();
                _worldAABB    = _transform.transformAABB(_geometricData->getBLAS(_meshId).rootBounds());
            }

            RayHit intersect(const Ray &worldRay) const noexcept {
                return _geometricData->getBLAS(_meshId).traverse(_invTransform.transformRay(worldRay));
            }

            const Triangle &getPrimitive(uint32_t idx) const noexcept {
                return _geometricData->getTriangles(_meshId)[idx];
            }

            const Math::Matrix4 &getTransform() const noexcept {return _transform;}
            const Math::AABB &worldAABB() const noexcept {return _worldAABB;}

            Graphic::Material &getMaterial() noexcept {return _material;}
            const Graphic::Material &getMaterial() const noexcept {return _material;}
            void setMaterial(const Graphic::Material &mat) noexcept {_material = mat;}

        private:
            const GeometricData *_geometricData;
            uint32_t _meshId;
            Math::Matrix4 _transform;
            Math::Matrix4 _invTransform;
            Math::AABB _worldAABB;
            Graphic::Material _material;
        };
}

#endif