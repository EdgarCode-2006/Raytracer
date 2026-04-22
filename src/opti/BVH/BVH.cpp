#include "opti/BVH/BVH.hpp"
#include "opti/BVH/BVHNode.hpp"
#include "utils/rayTracer.hpp"
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>


namespace RayTracer {

    uint32_t BVH::_allocNode() {
        _nodes.emplace_back();
        return static_cast<uint32_t>(_nodes.size() - 1);
    }

    BVH::BVH(const std::vector<Triangle> &triangles, int leafThreshold) : _triangles(&triangles), _leafThreshold(leafThreshold) {
        _build();
    }

    void BVH::_build() {
        const size_t n = _triangles->size();

        if (n == 0)
            return;

        _indices.resize(n);
        std::iota(_indices.begin(), _indices.end(), 0);

        _centroids.resize(n);
        _primAABBs.resize(n);
        for (size_t i = 0; i < n; i++) {
            const Triangle &tri = (*_triangles)[i];
            _centroids[i] = tri.centroid();
            _primAABBs[i].expand(tri.a());
            _primAABBs[i].expand(tri.b());
            _primAABBs[i].expand(tri.c());
        }

        _nodes.reserve(n * 2);
        _allocNode();
        _subdivide(0, 0, static_cast<uint32_t>(n), 0);
    }

    BVH::BoundsResult BVH::_computeBounds(uint32_t first, uint32_t count) const noexcept {
        BoundsResult result;

        for (uint32_t i = 0; i < count; i++) {
            const uint32_t primitiveIndex = _indices[first + i];
            result.nodeAABB.expand(_primAABBs[primitiveIndex]);
            result.centAABB.expand(_centroids[primitiveIndex]);
        }
        return result;
    }
}