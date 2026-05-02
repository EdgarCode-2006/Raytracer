#ifndef GEOMETRICDATA_HPP_
#define GEOMETRICDATA_HPP_

#include "opti/BVH/BVH.hpp"
#include "utils/rayTracer.hpp"
#include <cstdint>
#include <span>
#include <vector>

namespace RayTracer {

    struct MeshRange {
        uint32_t start;
        uint32_t count;
    };

    class GeometricData {
    public:
        GeometricData() = default;
        ~GeometricData() = default;

        GeometricData(const GeometricData &) = delete;
        GeometricData &operator=(const GeometricData &) = delete;
        GeometricData(GeometricData &&) = default;
        GeometricData &operator=(GeometricData &&) = default;

        void reserve(uint32_t meshCount, uint32_t totalTriangles);
        uint32_t addMesh(std::vector<Triangle> tris);
        std::span<const Triangle> getTriangles(uint32_t meshId) const noexcept;
        const std::vector<Triangle> &getAllTriangles() const noexcept {return _allTriangles;}
        const BVH &getBLAS(uint32_t meshId) const noexcept {return _blases[meshId];}
        uint32_t getMeshCount() const noexcept {return static_cast<uint32_t>(_meshRanges.size());}

    private:
        std::vector<Triangle>   _allTriangles;
        std::vector<MeshRange>  _meshRanges;
        std::vector<BVH>        _blases;
        // std::vector<Normal> _allNormals;
        // std::vector<UV>     _allUVs;
    };

}

#endif