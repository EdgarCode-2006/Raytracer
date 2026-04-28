#ifndef TLASNODE_HPP_
#define TLASNODE_HPP_

#include "opti/BVH/AABB.hpp"
#include <cstdint>

namespace RayTracer {
    class TLASNode {
        public:
            TLASNode() = default;
    
            bool isLeaf() const noexcept { return count > 0;}
    
            RayTracer::AABB bounds;
            uint32_t leftOrFirst = 0;
            uint32_t count = 0;
    };
}


#endif