#ifndef SCENE_HPP_
#define SCENE_HPP_
#include <cstdint>
#include <iterator>
#include <span>
#include <vector>

#include "entities/Camera.hpp"
#include "opti/BVH/BVH.hpp"
#include "opti/BVH/Instance.hpp"
#include "opti/BVH/TLAS.hpp"
#include "utils/Matrix4.hpp"
#include "utils/rayTracer.hpp"
#include "entities/Light.hpp"
#include <chrono>
#include <iostream>

namespace RayTracer {

    struct MeshRange {
        uint32_t start;
        uint32_t count;
    };

    class Scene {
        public:
            Scene(const int width, const int height);
            ~Scene() = default;

            Scene(const Scene&) = delete;
            Scene& operator=(const Scene&) = delete;
            Scene(Scene&&) = default;
            Scene& operator=(Scene&&) = default;

            void reserveMeshes(uint32_t meshCount, uint32_t totalTriangles, uint32_t totalInstances) {
                _allTriangles.reserve(totalTriangles);
                _meshRanges.reserve(meshCount);
                _blases.reserve(meshCount);
                _instances.reserve(totalInstances);
            }

            uint32_t addMesh(std::vector<Triangle> tris) {
                const uint32_t id = static_cast<uint32_t>(_meshRanges.size());
                const uint32_t start = static_cast<uint32_t>(_allTriangles.size());
                const uint32_t count = static_cast<uint32_t>(tris.size());

                _allTriangles.insert(_allTriangles.end(), std::make_move_iterator(tris.begin()), std::make_move_iterator(tris.end()));
                _meshRanges.push_back({start, count});

                const auto startChrono = std::chrono::high_resolution_clock::now();

                _blases.emplace_back(
                    std::span<const Triangle>(_allTriangles.data() + start, count)
                );
                const auto endChrono = std::chrono::high_resolution_clock::now();
                std::cout << "BVH build time: " << std::chrono::duration<double, std::milli>(endChrono - startChrono).count() << " ms\n";
                return id;
            }

            Instance &addInstance(uint32_t meshId, const Matrix4 &transform =  {}) {
                const MeshRange &r = _meshRanges[meshId];
                return  _instances.emplace_back(
                    &_blases[meshId],
                    _allTriangles.data() + r.start,
                    r.count,
                    transform
                );
            }

            void buildTLAS() {
                const auto startChrono = std::chrono::high_resolution_clock::now();

                _tlas.build(std::span<const Instance>(_instances));
                const auto endChrono = std::chrono::high_resolution_clock::now();

                std::cout << "TLAS build time: " << std::chrono::duration<double, std::milli>(endChrono - startChrono).count() << " ms\n";

            }

            const TLAS &getTLAS() const noexcept {return _tlas;}
            Instance& getInstance(uint32_t i) noexcept { return _instances[i]; }
            const Instance &getInstance(uint32_t i) const noexcept {return _instances[i];}

            std::span<const Triangle> getTriangles(uint32_t meshId) const noexcept {
                const MeshRange &r = _meshRanges[meshId];
                return {_allTriangles.data() + r.start, r.count};
            }

            const std::vector<Triangle> &getAllTriangles() const noexcept {
                return _allTriangles;
            }

            Camera& getCamera() noexcept {return _camera;};
            const Camera& getCamera() const noexcept {return _camera;};

            void addLight(const DirectionalLight& light);
            void setAmbientLight(const AmbientLight& light);
            const std::vector<DirectionalLight>& getLights() const { return _directionalLights; }
            const AmbientLight& getAmbientLight() const { return _ambientLight; }
            uint32_t getInstanceCount() {
                return this->_instances.size();
            }
            // void merge(const Scene& other);

        private:
            std::vector<Triangle> _allTriangles;
            std::vector<MeshRange> _meshRanges;
            std::vector<BVH> _blases;
            std::vector<Instance> _instances;
            TLAS _tlas;

            Camera _camera;
            std::vector<DirectionalLight> _directionalLights;
            AmbientLight _ambientLight;
    };
}
#endif
