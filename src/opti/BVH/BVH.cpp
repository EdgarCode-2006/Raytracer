#include "opti/BVH/BVH.hpp"
#include "opti/BVH/AABB.hpp"
#include "opti/BVH/BVHNode.hpp"
#include "opti/BVH/RayHit.hpp"
#include "utils/math.hpp"
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

    BVH::BVH(std::span<const Triangle> triangles, int leafThreshold) : _leafThreshold(leafThreshold) {
        _build(triangles);
    }

    void BVH::_build(std::span<const Triangle> triangles) {
        const size_t n = triangles.size();

        if (n == 0)
            return;

        _indices.resize(n);
        std::iota(_indices.begin(), _indices.end(), 0);

        _centroids.resize(n);
        _primAABBs.resize(n);
        for (size_t i = 0; i < n; i++) {
            const Triangle &tri = triangles[i];
            _centroids[i] = tri.centroid();
            _primAABBs[i].expand(tri.a());
            _primAABBs[i].expand(tri.b());
            _primAABBs[i].expand(tri.c());
        }

        _nodes.reserve(n * 2);
        _allocNode();
        _subdivide(0, 0, static_cast<uint32_t>(n), 0);

        // the traversal only needs the nodes and the indices
        _nodes.shrink_to_fit();
        std::vector<Math::Vector3D>().swap(_centroids);
        std::vector<Math::AABB>().swap(_primAABBs);
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

    bool BVH::_splitSAH(uint32_t first, uint32_t count, const Math::AABB &nodeBounds, const Math::AABB &centBounds, int &bestAxis, double &bestPos) const noexcept {
        struct Bucket {
            Math::AABB bounds;
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

            Math::AABB leftBounds[BUCKETS];
            int leftCount[BUCKETS] = {};
            Math::AABB left;
            int lc = 0;

            for (int i = 0; i < BUCKETS; i++) {
                left.expand(buckets[i].bounds);
                lc += buckets[i].count;
                leftBounds[i] = left;
                leftCount[i] = lc;
            }

            Math::AABB right;
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

    double BVH::_slabTest(const BVHNode& node, const double* ro, const double* invd, double tmax) noexcept {
        const Math::Vector3D boxMin = node.bounds().min();
        const Math::Vector3D boxMax = node.bounds().max();
        double tmin = 0.0;

        for (int i = 0; i < 3; i++) {
            double t1 = (boxMin[i] - ro[i]) * invd[i];
            double t2 = (boxMax[i] - ro[i]) * invd[i];

            // Ensure t1 = entry, t2 = exit
            if (t1 > t2) { 
                double tmp = t1;
                t1 = t2;
                t2 = tmp;
            }
             /*
                Intersect intervals:
                - latest entry time
                - earliest exit time
            */
            if (t1 > tmin)
                tmin = t1;
            if (t2 < tmax)
                tmax = t2;
        }
        // if intervals overlap, the ray intersects the box otherwise, it misses.
        return (tmin <= tmax) ? tmin : 1e30;
    }

    RayHit BVH::_mollerTrumbore(uint32_t primitiveIndex, const Triangle &tri, const Ray &ray) noexcept {
        const double EPSILON = 1e-7;
        Math::Vector3D edge1 = tri.b() - tri.a();
        Math::Vector3D edge2 = tri.c() - tri.a();
        Math::Vector3D h = ray.direction.cross(edge2);
        double a = edge1.dot(h);
        if (a > -EPSILON && a < EPSILON)
            return {};
        double f = 1.0 / a;
        Math::Vector3D s = ray.origin - tri.a();
        double u = f * s.dot(h);
        if (u < 0.0 || u > 1.0)
            return {};
        Math::Vector3D q = s.cross(edge1);
        double v = f * ray.direction.dot(q);
        if (v < 0.0 || u + v > 1.0)
            return {};
        double t = f * edge2.dot(q);
        return (t > EPSILON) ? RayHit(t, primitiveIndex, u, v) : RayHit{};
    }

    RayHit BVH::traverse(const Ray &ray, std::span<const Triangle> triangles, double tMax) const noexcept {
        if (_nodes.empty())
            return {};

        const double ro[3] = {
            ray.origin.x(),
            ray.origin.y(),
            ray.origin.z()
        };
        const double invd[3] = {
            1.0 / ray.direction.x(),
            1.0 / ray.direction.y(),
            1.0 / ray.direction.z(),
        };

        RayHit hit;
        double closest = tMax;

        // Each node keeps the entry distance computed when it was pushed: when it is popped,
        // comparing it to the closest hit gives the same answer as testing its box again.
        struct Entry {
            uint32_t node;
            double tEntry;
        };
        Entry stack[STACK_SIZE];
        int top = 0;

        const double tRoot = _slabTest(_nodes[0], ro, invd, closest);
        if (tRoot >= closest)
            return hit;
        stack[top++] = {0, tRoot};

        while (top > 0) {
            const Entry entry = stack[--top];

            if (entry.tEntry >= closest)
                continue;

            const BVHNode &node = _nodes[entry.node];

            if (node.isLeaf()) {
                for (uint32_t i = 0; i < node.primCount(); i++) {
                    const uint32_t primitiveIndex = _indices[node.leftOrFirst() + i];
                    const RayHit candidate = _mollerTrumbore(primitiveIndex, triangles[primitiveIndex], ray);
                    if (candidate.t() < closest) {
                        hit = candidate;
                        closest = candidate.t();
                    }
                }
            } else {
                // near first far last
                const uint32_t leftIndex = node.leftOrFirst();
                const uint32_t rightIndex = leftIndex + 1;

                const double tLeft = _slabTest(_nodes[leftIndex], ro, invd, closest);
                const double tRight = _slabTest(_nodes[rightIndex], ro, invd, closest);

                if (tLeft < tRight) {
                    if (tRight < closest)
                        stack[top++] = {rightIndex, tRight};
                    if (tLeft < closest)
                        stack[top++] = {leftIndex, tLeft};
                } else {
                    if (tLeft < closest)
                        stack[top++] = {leftIndex, tLeft};
                    if (tRight < closest)
                        stack[top++] = {rightIndex, tRight};
                }
            }
        }
        return hit;
    }

    bool BVH::occluded(const Ray &ray, std::span<const Triangle> triangles, double tMax) const noexcept {
        if (_nodes.empty())
            return false;

        const double ro[3] = {
            ray.origin.x(),
            ray.origin.y(),
            ray.origin.z()
        };
        const double invd[3] = {
            1.0 / ray.direction.x(),
            1.0 / ray.direction.y(),
            1.0 / ray.direction.z(),
        };

        uint32_t stack[STACK_SIZE];
        int top = 0;

        if (_slabTest(_nodes[0], ro, invd, tMax) >= tMax)
            return false;
        stack[top++] = 0;

        // any hit will do, no need to find the closest one
        while (top > 0) {
            const BVHNode &node = _nodes[stack[--top]];

            if (node.isLeaf()) {
                for (uint32_t i = 0; i < node.primCount(); i++) {
                    const uint32_t primitiveIndex = _indices[node.leftOrFirst() + i];
                    if (_mollerTrumbore(primitiveIndex, triangles[primitiveIndex], ray).t() < tMax)
                        return true;
                }
            } else {
                const uint32_t leftIndex = node.leftOrFirst();
                const uint32_t rightIndex = leftIndex + 1;

                if (_slabTest(_nodes[rightIndex], ro, invd, tMax) < tMax)
                    stack[top++] = rightIndex;
                if (_slabTest(_nodes[leftIndex], ro, invd, tMax) < tMax)
                    stack[top++] = leftIndex;
            }
        }
        return false;
    }
}