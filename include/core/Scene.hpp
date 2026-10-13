#ifndef SCENE_HPP_
#define SCENE_HPP_
#include <cstdint>
#include <iterator>
#include <map>
#include <span>
#include <vector>

#include "core/SceneComponents/GeometricData.hpp"
#include "core/SceneComponents/World.hpp"
#include "core/SceneComponents/Lights.hpp"
#include "entities/Camera.hpp"
#include "opti/BVH/BVH.hpp"
#include "opti/BVH/Instance.hpp"
#include "opti/BVH/TLAS.hpp"
#include "utils/Matrix4.hpp"
#include "utils/rayTracer.hpp"
#include "utils/Material.hpp"
#include "entities/Light.hpp"
#include "entities/HDRI.hpp"
#include "entities/Background.hpp"
#include <chrono>
#include <iostream>

namespace RayTracer {
    // What a frame is rendered from. The render threads only read the instances through the TLAS
    // (its own copy of them), so the game can move, add and remove instances while a frame renders.
    struct RenderScene {
        const GeometricData &geometry;
        std::shared_ptr<const TLAS> tlas;
        const Lights &lights;
        const Background &background;
        double backgroundTime;
    };

    class Scene {
        public:
            Scene(const int width, const int height) : _camera(width, height) {};
            ~Scene() = default;

            Scene(const Scene&) = delete;
            Scene &operator=(const Scene &) = delete;
            Scene(Scene &&) = delete;
            Scene &operator=(Scene &&) = default;

            void reserve(uint32_t meshCount, uint32_t totalTriangles, uint32_t totalInstances) {
                _geometry.reserve(meshCount, totalTriangles);
                _world.reserve(totalInstances);
            }

            uint32_t addMesh(std::vector<Triangle> tris, const std::string& path = "") {
                _meshPaths.push_back(path);
                return _geometry.addMesh(std::move(tris));
            }
            const std::string& getMeshPath(uint32_t meshId) const noexcept { return _meshPaths[meshId]; }

            Instance& addInstance(uint32_t meshId,
                const Math::Matrix4& transform = Math::Matrix4{},
                const Graphic::Material& mat = Graphic::Material{}) {
                return _world.addInstance(_geometry, meshId, transform, mat);
            }

            void buildTLAS() {
                _world.buildTLAS();
            }
            
            void loadHDRI(const std::string& path) { _lights.loadHDRI(path); }
            float &getEnvIntensity() {return _lights.getEnvIntensity();};
            void requestHDRILoad(const std::string& path) { _pendingHdriPath = path; }
            bool hasPendingHdri() const noexcept { return !_pendingHdriPath.empty(); }
            void applyPendingHdri() { _lights.loadHDRI(_pendingHdriPath); _pendingHdriPath.clear(); }
            const std::string& getHDRIPath() const noexcept { return _lights.getHDRIPath(); }
            GeometricData &getGeometry() noexcept {return _geometry;}
            const GeometricData &getGeometry() const noexcept {return _geometry;}
            
            World &getWorld() noexcept {return _world;}
            const World &getWorld() const noexcept {return _world;}

            void removeInstancesFrom(uint32_t first) {_world.removeInstancesFrom(first);}

            Instance &getInstance(uint32_t index) noexcept {return _world.getInstance(index);}
            const Instance &getInstance(uint32_t index) const noexcept {return _world.getInstance(index);}
            uint32_t getInstanceCount() const noexcept {return _world.getInstanceCount();}
            const TLAS &getTLAS() const noexcept {return _world.getTLAS();}


            Camera &getCamera() noexcept {return _camera;}
            const Camera &getCamera() const noexcept {return _camera;}

            Lights &getLights() noexcept {return _lights;}
            const Lights &getLights() const noexcept {return _lights;}

            void addLight(const DirectionalLight &light) {_lights.addLight(light);}
            void addPointLight(const PointLight &light) {_lights.addPointLight(light);}
            void setAmbientLight(const AmbientLight &light) {_lights.setAmbientLight(light);}
            const std::vector<DirectionalLight> &getDirectionalLights() const {return _lights.getDirectionalLights();}
            std::vector<DirectionalLight> &getDirectionalLights() {return _lights.getDirectionalLights();}
            const std::vector<PointLight> &getPointLights() const {return _lights.getPointLights();}
            std::vector<PointLight> &getPointLights() {return _lights.getPointLights();}
            AmbientLight &getAmbientLight() {return _lights.getAmbientLight();}
            const AmbientLight &getAmbientLight() const {return _lights.getAmbientLight();}
            const Graphic::Pixel getHDRIPixel(const Math::Vector3D &v) const {return _lights.getHDRIPixel(v);}

            Background &getBackground() noexcept {return _background;}
            const Background &getBackground() const noexcept {return _background;}
            void scrollBackground(double seconds) noexcept {_backgroundTime += seconds;}

            void setSavedFeature(const std::string& name, bool active) { _savedFeatures[name] = active; }
            const std::map<std::string, bool>& getSavedFeatures() const noexcept { return _savedFeatures; }


            inline std::shared_ptr<const RenderScene> generateSceneSnapShot() const {
                return std::make_shared<const RayTracer::RenderScene>(_geometry, _world.getTLASSnapshot(), _lights,
                    _background, _backgroundTime);
            }
        private:
            std::vector<std::string> _meshPaths;
            GeometricData _geometry;
            World _world;
            Camera _camera;
            Lights _lights;
            Background _background;
            double _backgroundTime = 0.0;
            std::map<std::string, bool> _savedFeatures;
            std::string _pendingHdriPath;
    };
}
#endif
