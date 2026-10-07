#ifndef BVH_HPP_
#define BVH_HPP_

#include "opti/BVH/AABB.hpp"
#include "opti/BVH/BVHNode.hpp"
#include "opti/BVH/RayHit.hpp"
#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include <cstdint>
#include <vector>
#include <span>
#define STACK_SIZE 64

namespace RayTracer {
    inline constexpr int BUCKETS = 32;
    inline constexpr double BVH_EPSILON = 1e-12; // si une boite est tellement petite qu'elle est plate c'est qu'il n'y a rien à optimiser
    inline constexpr double BVH_SPAN_EPSILON = 1e-9;
    inline constexpr double BVH_TRAVERSAL_COST = 0.125; // le cout de descendre dans un noued
    class BVH {
        public:
            // The triangles are only read during the build: the BVH keeps no pointer to them,
            // so the array that owns them can grow (and reallocate) afterwards.
            BVH(std::span<const Triangle> triangles, int leafThreshold = 4);

            ~BVH() = default;
            BVH(const BVH &) = delete;
            BVH &operator=(const BVH &) = delete;
            BVH(BVH &&) = default;
            BVH &operator=(BVH &&) = default;

            // Closest hit closer than tMax, `triangles` being the ones the BVH was built on.
            RayHit traverse(const Ray &ray, std::span<const Triangle> triangles, double tMax = RAY_NO_HIT) const noexcept;
            // Whether anything is hit closer than tMax: stops at the first hit (shadow rays).
            bool occluded(const Ray &ray, std::span<const Triangle> triangles, double tMax = RAY_NO_HIT) const noexcept;
            const Math::AABB &rootBounds() const noexcept {return _nodes[0].bounds();}

        private:

            struct BoundsResult {
                Math::AABB nodeAABB; // AABB of triangles (surface)
                Math::AABB centAABB; // AABB of centroids (for bucketing SAH)
            };

            void _build(std::span<const Triangle> triangles);
            uint32_t _allocNode();

            BoundsResult _computeBounds(uint32_t first, uint32_t count) const noexcept;
            void _subdivide(uint32_t nodeIndex, uint32_t first, uint32_t count, int depth);
            bool _splitSAH(uint32_t first, uint32_t count, const Math::AABB& nodeBounds, const Math::AABB& centBounds, int& bestAxis, double& bestPos) const noexcept;

            static double _slabTest(const BVHNode& node, const double* ro, const double* invd, double tmax) noexcept;
            static RayHit _mollerTrumbore(uint32_t primitiveIndex, const Triangle& tri, const Ray& ray) noexcept;

            std::vector<BVHNode> _nodes;
            std::vector<uint32_t> _indices;

            // only used while building, released afterwards
            std::vector<Math::Vector3D> _centroids;
            std::vector<Math::AABB> _primAABBs;

            int _leafThreshold;
        };
}


#endif
