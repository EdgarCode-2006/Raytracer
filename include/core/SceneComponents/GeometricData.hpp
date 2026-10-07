#ifndef GEOMETRICDATA_HPP_
#define GEOMETRICDATA_HPP_

#include "opti/BVH/BVH.hpp"
#include "utils/rayTracer.hpp"
#include <cstdint>
#include <span>
#include <vector>

namespace RayTracer {

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
  std::span<const Triangle> getTriangles(uint32_t meshId) const noexcept {
    const MeshRange &mesh = _meshes[meshId];
    return std::span<const Triangle>(_allTriangles).subspan(mesh.first, mesh.count);
  }
  const std::vector<Triangle> &getAllTriangles() const noexcept {
    return _allTriangles;
  }
  const BVH &getBLAS(uint32_t meshId) const noexcept { return _blases[meshId]; }
  uint32_t getMeshCount() const noexcept {
    return static_cast<uint32_t>(_meshes.size());
  }

private:
  struct MeshRange {
    size_t first;
    size_t count;
  };

  // Every triangle is stored once, mesh after mesh: a mesh is a range of this array.
  // Adding a mesh can reallocate it, so nothing keeps pointers into it between frames.
  std::vector<Triangle> _allTriangles;
  std::vector<MeshRange> _meshes;
  std::vector<BVH> _blases;
};

} // namespace RayTracer

#endif