#ifndef BVH_HPP_
#define BVH_HPP_

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
            void _build();
            uint32_t _allocNode();
            void _computeAABB(uint32_t nodeIndex, uint32_t first, uint32_t count);
            void _subdivide(uint32_t nodeIndex, uint32_t first, uint32_t count, int depth);
            void _splitSAH(uint32_t first, uint32_t count, int &bestAxis, double &bestPos);

            const std::vector<Triangle>* _triangles;

            std::vector<uint32_t> _indices;
            std::vector<Math::Vector3D> _centroids;

            int _leafThreshold;
        };
}


#endif