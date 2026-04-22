#include "opti/BVH/BVHNode.hpp"
#include "opti/BVH/AABB.hpp"
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

    const AABB &BVHNode::bounds() const noexcept  {
        return _bounds;
    }
}