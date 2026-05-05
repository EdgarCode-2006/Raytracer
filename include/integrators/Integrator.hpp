#ifndef INTEGRATOR_HPP_
#define INTEGRATOR_HPP_
#include <memory>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <memory>

#include "core/Scene.hpp"
#include <raylib.h>
#include "integrators/integratorElements/PixelTile.hpp"
<<<<<<< HEAD
=======
#include "opti/BVH/BVH.hpp"

>>>>>>> 8a63244 (chore: post-rebase build fixes and cleanup)
#include "opti/ThreadPool/threadPool.hpp"
#include "utils/Pixel.hpp"
#include "utils/surfaceIntersection.hpp"
#include "integrators/rayFeature/IRayFeature.hpp"
#include "integrators/featureLoader/DLLoader.hpp"
#include "integrators/integratorElements/frame.hpp"
#include <memory>
#include <tuple>

#define TILE_DIM 64
#define MIN_THROUGHPUT_LENGTH 0.001
#define RAYS_PER_PIXEL 8
#define MAX_DEPTH 5


namespace RayTracer {
    class IRayFeature;

    struct CachedData {
        std::vector<PixelTile> tiles;
        int samplesPerSide;
        float step;
    };

    class Integrator {
        public:
            Integrator(std::shared_ptr<const RenderScene> scene);
            ~Integrator();
            void renderScene(const Camera &cam);
            void instanceTiles(const Camera &cam);
            Graphic::Pixel shootRay(Ray &ray, std::shared_ptr<const RenderScene> sceneLocked);
            void waitForRendering() {_threadPool->wait();};
            void stop() { _stopRendering = true; };
            std::shared_ptr<Frame> getAvailableFrame();
            void setCurrentSceneSnapshot(std::shared_ptr<const RenderScene> scene) { this->_scene = std::move(scene); };
            void loadRayFeatures();
            std::vector<std::unique_ptr<IRayFeature>>& getPipeline() noexcept { return _pipeline; }
            
        private:
            Graphic::Pixel consumeRays(Ray &ray, std::shared_ptr<const RenderScene> scene, SurfaceIntersection &intersection);
            void computeTile(const PixelTile &tile, std::shared_ptr<Frame> currentFrame, std::shared_ptr<const RenderScene> sceneLocked, const Camera &cam);
            void setPluginState(const std::string &pluginName, const bool &state);
            Graphic::Pixel getPixelColor(std::tuple<int, int> cords, std::shared_ptr<const RenderScene> sceneLocked, const Camera &cam);

            std::shared_ptr<const RenderScene> _scene; // current Scene snapshot.

            std::unique_ptr<optimization::ThreadPool> _threadPool;
            bool _stopRendering = false;

            std::vector<DLLoader<IRayFeature>> _loaders;
            std::vector<std::unique_ptr<IRayFeature>> _pipeline;

            std::queue<std::shared_ptr<Frame>> _readyPixelBuffers; // frame ready to be rendered by main thread.
            std::mutex _queueMutex;
            std::atomic<int> _framesInFlight;

            // cached data.
            CachedData _cache;
    };

    class IntegratorException : public std::exception {
        public:
            IntegratorException(const std::string& message) : _message(message) {}
            const char* what() const noexcept override { return _message.c_str(); }
        private:
            std::string _message;
    };
}
#endif
