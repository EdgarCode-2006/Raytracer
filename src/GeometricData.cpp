

#include "GeometricData.hpp"
namespace RayTracer {
    void GeometricData::reserve(uint32_t meshCount, uint32_t totalTriangles) {
        _allTriangles.reserve(totalTriangles);
        _meshRanges.reserve(meshCount);
        _blases.reserve(meshCount);
    }

    uint32_t GeometricData::addMesh(std::vector<Triangle> tris) {
        const uint32_t id = static_cast<uint32_t>(_meshRanges.size());
        const uint32_t start = static_cast<uint32_t>(_allTriangles.size());
        const uint32_t count = static_cast<uint32_t>(tris.size());

        _allTriangles.insert(_allTriangles.end(),
            std::make_move_iterator(tris.begin()),
            std::make_move_iterator(tris.end()));

        _meshRanges.push_back({start, count});
        _blases.emplace_back(
            std::span<const Triangle>(_allTriangles.data() + start, count)
        );

        return id;
    }

    std::span<const Triangle> GeometricData::getTriangles(uint32_t meshId) const noexcept {
        const MeshRange &r = _meshRanges[meshId];
        return {_allTriangles.data() + r.start, r.count};
    }
}