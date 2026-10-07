
#include "core/SceneComponents/World.hpp"
#include "opti/BVH/Instance.hpp"
namespace RayTracer {
    Instance &World::addInstance(const GeometricData& geo, uint32_t meshId, const Math::Matrix4 &transform, const Graphic::Material& mat) {
        Instance &inst = _instances.emplace_back(&geo, meshId, transform);
        inst.setMaterial(mat);
        return inst;
    }
}