#ifndef BVH_HPP_
#define BVH_HPP_

#include "opti/BVH/AABB.hpp"
#include "opti/BVH/BVHNode.hpp"
#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include <cstdint>
#include <vector>

namespace RayTracer {
    inline constexpr int BUCKETS = 32;
    inline constexpr double BVH_EPSILON = 1e-12; // si une boite est tellement petite qu'elle est plate c'est qu'il n'y a rien à optimiser
    inline constexpr double BVH_SPAN_EPSILON = 1e-9;
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
            bool _splitSAH(uint32_t first, uint32_t count, const AABB& nodeBounds, const AABB& centBounds, int& bestAxis, double& bestPos) const noexcept;

            const std::vector<Triangle>* _triangles;
            std::vector<BVHNode> _nodes;
            std::vector<uint32_t> _indices;
            std::vector<Math::Vector3D> _centroids;
            std::vector<AABB> _primAABBs;

            int _leafThreshold;
        };
}


#endif