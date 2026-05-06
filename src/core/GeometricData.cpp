

#include "core/SceneComponents/GeometricData.hpp"
namespace RayTracer {
void GeometricData::reserve(uint32_t meshCount, uint32_t totalTriangles) {
  _meshTriangles.reserve(meshCount);
  _allTriangles.reserve(totalTriangles);
  _blases.reserve(meshCount);
}

uint32_t GeometricData::addMesh(std::vector<Triangle> tris) {
  const uint32_t id = static_cast<uint32_t>(_meshTriangles.size());

  _allTriangles.insert(_allTriangles.end(), tris.begin(), tris.end());
  _meshTriangles.push_back(std::move(tris));
  _blases.emplace_back(std::span<const Triangle>(_meshTriangles.back()));

  return id;
}

std::span<const Triangle>
GeometricData::getTriangles(uint32_t meshId) const noexcept {
  return _meshTriangles[meshId];
}
} // namespace RayTracer