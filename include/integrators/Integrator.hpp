#ifndef INTEGRATOR_HPP_
#define INTEGRATOR_HPP_
#include <memory>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <tuple>
#include <raylib.h>

#include "core/Scene.hpp"
#include "integrators/integratorElements/PixelTile.hpp"
#include "opti/ThreadPool/threadPool.hpp"
#include "utils/Pixel.hpp"
#include "utils/UV.hpp"
#include "utils/surfaceIntersection.hpp"
#include "integrators/rayFeature/IRayFeature.hpp"
#include "integrators/featureLoader/DLLoader.hpp"
#include "integrators/integratorElements/frame.hpp"
#include "integrators/featureLoader/PipelineManager.hpp"

#define TILE_DIM 16
#define MIN_THROUGHPUT_LENGTH 0.001
#define MAX_DEPTH 5
#define DEFAULT_SAMPLES_PER_PIXEL 64

namespace RayTracer {
    class IRayFeature;

    struct CachedData {
        std::vector<PixelTile> tiles;
        Math::Vector3D prevCamPos;
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
            std::vector<std::unique_ptr<IRayFeature>>& getPipeline() noexcept { return _pipelineManager->_pipeline; };

        private:
            Graphic::Pixel consumeRays(Ray &ray, std::shared_ptr<const RenderScene> scene, SurfaceIntersection &intersection);
            void computeTile(const PixelTile &tile, std::shared_ptr<Frame> currentFrame, std::shared_ptr<const RenderScene> sceneLocked, const Camera &cam);
            Graphic::Pixel getPixelColor(std::tuple<int, int> cords, std::shared_ptr<const RenderScene> sceneLocked, const Camera &cam);
            void executePipeline(std::shared_ptr<const RenderScene> scene, SurfaceIntersection &intersection, ShaderType type);

            std::shared_ptr<const RenderScene> _scene; // current Scene snapshot.

            std::unique_ptr<optimization::ThreadPool> _threadPool;
            bool _stopRendering = false;

            std::unique_ptr<PipelineManager> _pipelineManager;

            std::queue<std::shared_ptr<Frame>> _readyPixelBuffers; // frame ready to be rendered by main thread.
            std::mutex _queueMutex;
            std::atomic<int> _framesInFlight;

            int _samplesPerPixel;

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
