#ifndef SCENE_HPP_
#define SCENE_HPP_
#include <cstdint>
#include <iterator>
#include <span>
#include <vector>

#include "GeometricData.hpp"
#include "World.hpp"
#include "Lights.hpp"
#include "entities/Camera.hpp"
#include "opti/BVH/BVH.hpp"
#include "opti/BVH/Instance.hpp"
#include "opti/BVH/TLAS.hpp"
#include "utils/Matrix4.hpp"
#include "utils/rayTracer.hpp"
#include "utils/Material.hpp"
#include "entities/Light.hpp"
#include "entities/HDRI.hpp"
#include <chrono>
#include <iostream>

namespace RayTracer {
    struct RenderScene {
        const GeometricData &geometry;
        const World &world;
        const Lights &lights;
    };

    class Scene {
        public:
            Scene(const int width, const int height) : _camera(width, height) {};
            ~Scene() = default;

            Scene(const Scene&) = delete;
            Scene &operator=(const Scene &) = delete;
            Scene(Scene &&) = default;
            Scene &operator=(Scene &&) = default;

            void reserve(uint32_t meshCount, uint32_t totalTriangles, uint32_t totalInstances) {
                _geometry.reserve(meshCount, totalTriangles);
                _world.reserve(totalInstances);
            }

            uint32_t addMesh(std::vector<Triangle> tris) {return _geometry.addMesh(std::move(tris));}

            Instance& addInstance(uint32_t meshId,
                const Math::Matrix4& transform = Math::Matrix4{},
                const Graphic::Material& mat = Graphic::Material{}) {
                return _world.addInstance(_geometry, meshId, transform, mat);
            }

            void buildTLAS() {
                _world.buildTLAS();
            }
            
            void loadHDRI(const std::string& path) { _lights.loadHDRI(path); }
            GeometricData &getGeometry() noexcept {return _geometry;}
            const GeometricData &getGeometry() const noexcept {return _geometry;}
            
            World &getWorld() noexcept {return _world;}
            const World &getWorld() const noexcept {return _world;}

            Instance &getInstance(uint32_t index) noexcept {return _world.getInstance(index);}
            const Instance &getInstance(uint32_t index) const noexcept {return _world.getInstance(index);}
            uint32_t getInstanceCount() const noexcept {return _world.getInstanceCount();}
            const TLAS &getTLAS() const noexcept {return _world.getTLAS();}


            Camera &getCamera() noexcept {return _camera;}
            const Camera &getCamera() const noexcept {return _camera;}

            Lights &getLights() noexcept {return _lights;}
            const Lights &getLights() const noexcept {return _lights;}

            void addLight(const DirectionalLight &light) {_lights.addLight(light);}
            void setAmbientLight(const AmbientLight &light) {_lights.setAmbientLight(light);}
            const std::vector<DirectionalLight> &getDirectionalLights() const {return _lights.getLights();}
            const AmbientLight &getAmbientLight() const {return _lights.getAmbientLight();}
            const Graphic::Pixel getHDRIPixel(const Math::Vector3D &v) const {return _lights.getHDRIPixel(v);}

            void setSelectedInstance(int32_t i) noexcept  {_selectedInstance = i;}
            int32_t getSelectedInstance() const noexcept  {return _selectedInstance;}

            inline std::shared_ptr<const RenderScene> generateSceneSnapShot() const {
                // std::cout << "instance 0 changed? " << this->_world. 
                return std::make_shared<const RayTracer::RenderScene>(_geometry, _world, _lights);
            }
        private:
            GeometricData _geometry;
            World _world;
            Camera _camera;
            Lights _lights;
            int32_t _selectedInstance = -1;
    };
}
#endif
