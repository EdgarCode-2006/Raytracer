#include "opti/BVH/RayHit.hpp"
#include "opti/BVH/TLASHit.hpp"
#include "opti/BVH/TLASNode.hpp"
#include <cstdint>
#include <sys/types.h>
#include "opti/BVH/TLAS.hpp"

namespace RayTracer {

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