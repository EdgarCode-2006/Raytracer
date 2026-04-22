#ifndef BVHNODE_HPP_
#define BVHNODE_HPP_

#include "opti/BVH/AABB.hpp"
#include <cstdint>
namespace RayTracer {

    class BVH;

    class BVHNode {
        public:
            BVHNode() = default;

            bool isLeaf() const noexcept;
            uint32_t leftOrFirst() const noexcept;
            uint32_t primCount() const noexcept;
            AABB bounds() const noexcept;
            
        private:
            double _aabbMin[3] = {0};
            uint32_t _leftorFirst = 0;
            double _aabbMax[3] = {0};
            uint32_t _primCount = 0;

            friend class BVH;
    };
}


#endif