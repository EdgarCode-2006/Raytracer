

#include "core/SceneComponents/GeometricData.hpp"
namespace RayTracer {
void GeometricData::reserve(uint32_t meshCount, uint32_t totalTriangles) {
  _meshes.reserve(meshCount);
  _allTriangles.reserve(totalTriangles);
  _blases.reserve(meshCount);
}

uint32_t GeometricData::addMesh(std::vector<Triangle> tris) {
  const uint32_t id = static_cast<uint32_t>(_meshes.size());
  const size_t first = _allTriangles.size();

  // first mesh: take its buffer instead of copying it, unless more room was reserved
  if (_allTriangles.empty() && tris.capacity() >= _allTriangles.capacity())
    _allTriangles = std::move(tris);
  else
    _allTriangles.insert(_allTriangles.end(), tris.begin(), tris.end());

  _meshes.push_back({first, _allTriangles.size() - first});
  _blases.emplace_back(getTriangles(id));

  return id;
}
} // namespace RayTracer
