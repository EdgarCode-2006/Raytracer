#include "opti/BVH/AABB.hpp"
#include "opti/BVH/Instance.hpp"
#include "opti/BVH/RayHit.hpp"
#include "opti/BVH/TLASHit.hpp"
#include "opti/BVH/TLASNode.hpp"
#include "utils/math.hpp"
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <span>
#include <sys/types.h>
#include "opti/BVH/TLAS.hpp"

namespace RayTracer {
    
    
    uint32_t TLAS::_expandBits(uint32_t v) noexcept {
        v &= 0x000003FFu; // garde les 10 bits utiles
        v = (v | (v << 16)) & 0x030000FFu;
        v = (v | (v <<  8)) & 0x0300F00Fu; // spread sur les octets
        v = (v | (v <<  4)) & 0x030C30C3u; // spread encore
        v = (v | (v <<  2)) & 0x09249249u; // un bit sur trois
        return v;
    }

    uint32_t TLAS::_mortonCode(double x, double y, double z) noexcept {
        const auto q = [](double v) -> uint32_t {
            const double clamped = v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v);
            return static_cast<uint32_t>(clamped * 1023.0);
        };
        return (_expandBits(q(x)) << 2) |
            (_expandBits(q(y)) << 1) |
                _expandBits(q(z));
    }
    
    void TLAS::build(const std::span<const Instance> instances) {
        _instances = instances;
        _nodes.clear();
        _mortonCodes.clear();

        const uint32_t instanceCount = static_cast<uint32_t>(_instances.size());

        if (instanceCount == 0)
            return;

        Math::AABB sceneBounds;
        for (const auto &instance : _instances)
            sceneBounds.expand(instance.worldAABB().center());
        
        const Math::Vector3D sceneMin = sceneBounds.min();
        const Math::Vector3D sceneSize = sceneBounds.size();

        _indices.resize(instanceCount);
        std::iota(_indices.begin(), _indices.end(), 0);
        _mortonCodes.resize(instanceCount);

        // compute morton code
        for (uint32_t i = 0; i < instanceCount; i++) {
            const Math::Vector3D c = _instances[i].worldAABB().center();

            const double nx = sceneSize.x() > 1e-9 ? (c.x() - sceneMin.x()) / sceneSize.x() : 0.5;
            const double ny = sceneSize.y() > 1e-9 ? (c.y() - sceneMin.y()) / sceneSize.y() : 0.5;
            const double nz = sceneSize.z() > 1e-9 ? (c.z() - sceneMin.z()) / sceneSize.z() : 0.5;

            _mortonCodes[i] = _mortonCode(nx, ny, nz);
        }

        std::sort(_indices.begin(), _indices.end(), [this](uint32_t a, uint32_t b) {
            return _mortonCodes[a] < _mortonCodes[b];
        });

        _nodes.reserve(instanceCount * 2);
        _allocNode();
        _buildLBVH(0, 0, instanceCount - 1);
    }

    uint32_t TLAS::_findSplit(uint32_t first, uint32_t last) const noexcept {
        const uint32_t firstCode = _mortonCodes[_indices[first]];
        const uint32_t lastCode  = _mortonCodes[_indices[last]];

        if (firstCode == lastCode)
            return (first + last) >> 1;

        const int commonPrefix = __builtin_clz(firstCode ^ lastCode);

        uint32_t split = first;
        uint32_t step  = last - first;

        do {
            step = (step + 1) >> 1;
            const uint32_t candidate = split + step;
            if (candidate < last) {
                const uint32_t candidateCode = _mortonCodes[_indices[candidate]];
                const int candidatePrefix = __builtin_clz(firstCode ^ candidateCode);
                if (candidatePrefix > commonPrefix)
                    split = candidate;
            }
        } while (step > 1);

        return split;
    }


    void TLAS::_buildLBVH(uint32_t nodeIndex, uint32_t first, uint32_t last) {
        // 1 object
        if (first == last) {
            _nodes[nodeIndex].leftOrFirst = first;
            _nodes[nodeIndex].count = 1;
            _nodes[nodeIndex].bounds = _instances[_indices[first]].worldAABB();
            return;
        }

        // 2 objects
        if (last == first + 1) {
            const uint32_t left = _allocNode();
            const uint32_t right = _allocNode();
            _nodes[nodeIndex].leftOrFirst = left;
            _nodes[nodeIndex].count = 0;
            _buildLBVH(left, first, first);
            _buildLBVH(right, last, last);
            Math::AABB box;
            box.expand(_nodes[left].bounds);
            box.expand(_nodes[right].bounds);
            _nodes[nodeIndex].bounds = box;
            return;
        }

        const uint32_t split = _findSplit(first, last);
        const uint32_t left = _allocNode();
        const uint32_t right = _allocNode();
        _nodes[nodeIndex].leftOrFirst = left;
        _nodes[nodeIndex].count = 0;

        _buildLBVH(left, first, split);
        _buildLBVH(right, split + 1, last);

        Math::AABB box;
        box.expand(_nodes[left].bounds);
        box.expand(_nodes[right].bounds);
        _nodes[nodeIndex].bounds = box;

    }

    uint32_t TLAS::_allocNode() {
        _nodes.emplace_back();
        return static_cast<uint32_t>(_nodes.size() - 1);
    }

    double TLAS::_slabTest(const TLASNode& node, const double* ro, const double* invd, double tmax) noexcept {
        const Math::Vector3D boxMin = node.bounds.min();
        const Math::Vector3D boxMax = node.bounds.max();
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

    TLASHit TLAS::traverse(const Ray &ray) const noexcept{
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

        TLASHit hit;
        uint32_t stack[STACK_SIZE]; 
        int top = 0;
        stack[top++] = 0; // root

        while (top > 0) {
            const TLASNode &node = _nodes[stack[--top]];

            if (_slabTest(node, ro, invd, hit.t()) >= hit.t())
                continue;
            
            if (node.isLeaf()) {
                for (uint32_t i = 0; i < node.count; i++) {
                    const uint32_t instanceIndex = _indices[node.leftOrFirst + i];
                    const RayHit instanceHit = _instances[instanceIndex].intersect(ray);
                    if (instanceHit.hasHit() && instanceHit.t() < hit.t())
                        hit = TLASHit(instanceHit.t(), instanceHit.primitiveIndex(), instanceIndex);
                }
            } else {
                const uint32_t leftIndex = node.leftOrFirst;
                const uint32_t rightIndex = leftIndex + 1;

                const double tLeft = _slabTest(_nodes[leftIndex], ro, invd, hit.t());
                const double tRight = _slabTest(_nodes[rightIndex], ro, invd, hit.t());

                if (tLeft < tRight) {
                    if (tRight < hit.t())
                        stack[top++] = rightIndex;
                    if (tLeft < hit.t())
                        stack[top++] = leftIndex;
                } else {
                    if (tLeft < hit.t())
                        stack[top++] = leftIndex;
                    if (tRight < hit.t())
                        stack[top++] = rightIndex;
                }
            }
        }
        return hit;
    }
}