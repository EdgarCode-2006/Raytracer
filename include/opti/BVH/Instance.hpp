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
#include <numbers>

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

            void setTRS(const float pos[3], const float rot[3], const float scale[3]) noexcept {
                _pos[0] = pos[0]; _pos[1] = pos[1]; _pos[2] = pos[2];
                _rot[0] = rot[0]; _rot[1] = rot[1]; _rot[2] = rot[2];
                _scale[0] = scale[0]; _scale[1] = scale[1]; _scale[2] = scale[2];
                const double d = std::numbers::pi / 180.0;
                Math::Matrix4 T = Math::Matrix4{}.translation(pos[0], pos[1], pos[2]);
                Math::Matrix4 Rx = Math::Matrix4{}.rotationX(rot[0] * d);
                Math::Matrix4 Ry = Math::Matrix4{}.rotationY(rot[1] * d);
                Math::Matrix4 Rz = Math::Matrix4{}.rotationZ(rot[2] * d);
                Math::Matrix4 S = Math::Matrix4{}.scale(scale[0], scale[1], scale[2]);
                setTransform(T * Ry * Rx * Rz * S);
            }

            RayHit intersect(const Ray &worldRay) const noexcept {
                return _geometricData->getBLAS(_meshId).traverse(_invTransform.transformRay(worldRay));
            }

            const Triangle &getPrimitive(uint32_t idx) const noexcept {
                return _geometricData->getTriangles(_meshId)[idx];
            }

            const Math::Matrix4 &getTransform() const noexcept {return _transform;}
            const Math::AABB &worldAABB() const noexcept {return _worldAABB;}

            const float* getPos() const noexcept { return _pos; }
            const float* getRot() const noexcept { return _rot; }
            const float* getScale() const noexcept { return _scale; }

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
            float _pos[3] = {0, 0, 0};
            float _rot[3] = {0, 0, 0};
            float _scale[3] = {1, 1, 1};
        };
}

#endif