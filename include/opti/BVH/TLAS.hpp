#ifndef TLAS_HPP_
#define TLAS_HPP_

#include "opti/BVH/Instance.hpp"
#include "opti/BVH/TLASHit.hpp"
#include "opti/BVH/TLASNode.hpp"
#include <cstdint>
#include <span>
#include <vector>


namespace RayTracer {
    class TLAS {
        public:
            TLAS() = default;
            
            void build(std::span<const Instance> instances);
            // Closest hit closer than tMax.
            TLASHit traverse(const Ray &ray, double tMax = RAY_NO_HIT) const noexcept;
            // Whether anything is hit closer than tMax: stops at the first hit (shadow rays).
            bool occluded(const Ray &ray, double tMax = RAY_NO_HIT) const noexcept;

        private:

            static uint32_t _expandBits(uint32_t v) noexcept;
            static uint32_t _mortonCode(double x, double y, double z) noexcept;

            uint32_t _allocNode();
            uint32_t _findSplit(uint32_t first, uint32_t last) const noexcept;
            void _buildLBVH(uint32_t nodeIndex, uint32_t first, uint32_t last);

            static double _slabTest(const TLASNode &node, const double *ro, const double *invd, double tmax) noexcept;

            std::vector<Instance> _instances;
            std::vector<uint32_t> _indices;
            std::vector<uint32_t> _mortonCodes;
            std::vector<TLASNode> _nodes;
    };
}



#endif