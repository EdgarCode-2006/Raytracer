#include "opti/BVH/BVHNode.hpp"
#include "opti/BVH/AABB.hpp"
#include "utils/math.hpp"
#include <cstdint>


namespace RayTracer {
    bool BVHNode::isLeaf() const noexcept {
        return _primCount > 0;
    }

    uint32_t BVHNode::leftOrFirst() const noexcept {
        return _leftorFirst;
    }
    
    uint32_t BVHNode::primCount() const noexcept {
        return _primCount;
    }

    AABB BVHNode::bounds() const noexcept  {
        AABB box;

        box.expand(Math::Vector3D(_aabbMin[0], _aabbMin[1], _aabbMin[2]));
        box.expand(Math::Vector3D(_aabbMax[0], _aabbMax[1], _aabbMax[2]));
        return box;
    }
}