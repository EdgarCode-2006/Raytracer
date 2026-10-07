#ifndef BVHNODE_HPP_
#define BVHNODE_HPP_

#include "opti/BVH/AABB.hpp"
#include <cstdint>
namespace RayTracer {

    class BVH;

    class BVHNode {
        public:
            BVHNode() = default;

            // Defined here so they inline into the traversal loops.
            bool isLeaf() const noexcept { return _primCount > 0; }
            uint32_t leftOrFirst() const noexcept { return _leftorFirst; }
            uint32_t primCount() const noexcept { return _primCount; }
            const Math::AABB &bounds() const noexcept { return _bounds; }

        private:
            Math::AABB _bounds;
            uint32_t _leftorFirst = 0;
            uint32_t _primCount = 0;

            friend class BVH;
    };
}


#endif