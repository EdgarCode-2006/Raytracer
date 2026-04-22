#ifndef BVH_HPP_
#define BVH_HPP_

#include "opti/BVH/BVHNode.hpp"
#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include <cstdint>
#include <vector>
namespace RayTracer {
    class BVH {
        public:
            BVH(const std::vector<Triangle> &triangles, int leafThreshold = 4);
            
            ~BVH() = default;
            BVH(const BVH &) = delete;
            BVH &operator=(const BVH &) = delete;
            BVH(BVH &&) = default;
            BVH &operator=(BVH &&) = default;

        private:

            struct BoundsResult {
                AABB nodeAABB; // AABB of triangles (surface)
                AABB centAABB; // AABB of centroids (for bucketing SAH)
            };

            void _build();
            uint32_t _allocNode();
            
            BoundsResult _computeBounds(uint32_t first, uint32_t count) const noexcept;
            void _subdivide(uint32_t nodeIndex, uint32_t first, uint32_t count, int depth);
            void _splitSAH(uint32_t first, uint32_t count, const AABB &centBounds,int &bestAxis, double &bestPos);

            const std::vector<Triangle>* _triangles;
            std::vector<BVHNode> _nodes;
            std::vector<uint32_t> _indices;
            std::vector<Math::Vector3D> _centroids;
            std::vector<AABB> _primAABBs;

            int _leafThreshold;
        };
}


#endif