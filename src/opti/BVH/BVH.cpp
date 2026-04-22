#include "opti/BVH/BVH.hpp"
#include "utils/rayTracer.hpp"
#include <cstddef>
#include <numeric>
#include <vector>


namespace RayTracer {
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
        for (size_t i = 0; i < n; i++)
            _centroids[i] = (*_triangles)[i].centroid();

    }
}