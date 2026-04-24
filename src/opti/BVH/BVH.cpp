#include "opti/BVH/BVH.hpp"
#include "opti/BVH/AABB.hpp"
#include "opti/BVH/BVHNode.hpp"
#include "utils/rayTracer.hpp"
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <sys/types.h>
#include <utility>
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

    void BVH::_subdivide(uint32_t nodeIndex, uint32_t first, uint32_t count, int depth) {
        auto [nodeBox, centBox] = _computeBounds(first, count);
        _nodes[nodeIndex]._bounds = nodeBox;

        if (static_cast<int>(count) <= _leafThreshold) {
            _nodes[nodeIndex]._leftorFirst = first;
            _nodes[nodeIndex]._primCount = count;
            return;
        }

        int axis = 0;
        double pos = 0.0;

        if (!_splitSAH(first, count, nodeBox, centBox, axis, pos)) {
            _nodes[nodeIndex]._leftorFirst = first;
            _nodes[nodeIndex]._primCount = count;
            return;
        }

        uint32_t mid = first;

        for (uint32_t i = first; i < first + count; i++)
            if (_centroids[_indices[i]][axis] < pos)
                std::swap(_indices[i], _indices[mid++]);
        
        // si tout est à gauche ou tout est à droite split inutile
        if (mid == first || mid == first + count) {
            _nodes[nodeIndex]._leftorFirst = first;
            _nodes[nodeIndex]._primCount = count;
            return;
        }

        const uint32_t leftIdx = _allocNode();
        const uint32_t rightIdx = _allocNode(); // rightIdx = leftIdx + 1

        _nodes[nodeIndex]._leftorFirst = leftIdx;
        _nodes[nodeIndex]._primCount = 0;

        // creation de l'arbre binaire
        _subdivide(leftIdx, first, mid - first, depth + 1);
        _subdivide(rightIdx, mid, (first + count) - mid, depth + 1);
    }

    bool BVH::_splitSAH(uint32_t first, uint32_t count, const AABB &nodeBounds, const AABB &centBounds, int &bestAxis, double &bestPos) const noexcept {
        struct Bucket {
            AABB bounds;
            int count = 0;
        };

        const double parentSA = nodeBounds.surfaceArea();

        if (parentSA < BVH_EPSILON)
            return false;
        
        double bestCost = static_cast<double>(count); // coût de base = rester une feuille
        bestAxis = -1;

        for (int axis = 0; axis < 3; axis++) {
            const double axMin = centBounds.min()[axis];
            const double axMax = centBounds.max()[axis];
            const double span = axMax - axMin;
            if (span < BVH_SPAN_EPSILON) // les centroides sont quasiment tous alignés sur cette axes donc impossible de faire une coupe utile
                continue;
        
            Bucket buckets[BUCKETS] = {};

            for (uint32_t i = 0; i < count; i++) {
                const uint32_t primitiveIdx = _indices[first + i];
                // normaliser la position du centroide sur un axe pour avoir une valeur entre 0 et 1 pour pouvoir avoir l'index du bucket
                const int bucketIdx = std::min(static_cast<int>((_centroids[primitiveIdx][axis] - axMin) / span * BUCKETS), BUCKETS -1);
                buckets[bucketIdx].bounds.expand(_primAABBs[primitiveIdx]);
                buckets[bucketIdx].count++;
            }

            AABB leftBounds[BUCKETS];
            int leftCount[BUCKETS] = {};
            AABB left;
            int lc = 0;

            for (int i = 0; i < BUCKETS; i++) {
                left.expand(buckets[i].bounds);
                lc += buckets[i].count;
                leftBounds[i] = left;
                leftCount[i] = lc;
            }

            AABB right;
            int rc = 0;
            for (int split = BUCKETS - 1; split >= 1; split--) {
                right.expand(buckets[split].bounds);
                rc += buckets[split].count;

                if (rc == 0 || leftCount[split - 1] == 0)
                    continue;

                const double cost = 0.125  + (leftBounds[split -1].surfaceArea() * leftCount[split - 1] + right.surfaceArea() * rc) / parentSA;

                if (cost < bestCost) {
                    bestCost = cost;
                    bestAxis = axis;
                    // convertir le bucket index en position réelle dans l'espace
                    // split est dans [0, BUCKETS] donc on le normalise et on le remappe dans l'intervalle [axMin, axMax]
                    bestPos = axMin + span * static_cast<double>(split) / BUCKETS;
                }
            }
        }

        return bestAxis >= 0;
    }
}