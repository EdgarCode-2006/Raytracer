#ifndef INTEGRATOR_HPP_
#define INTEGRATOR_HPP_
#include <array>
#include <memory>
#include <string>
#include <vector>
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
        Math::Vector3D prevCamForward;
    };

    class Integrator {
        public:
            Integrator(std::shared_ptr<const RenderScene> scene);
            ~Integrator();
            void renderScene(const Camera &cam);
            void instanceTiles(const Camera &cam);
            Graphic::Pixel shootRay(SurfaceIntersection &intersection, const RenderScene &scene, std::vector<Ray> &rayQueue);
            void waitForRendering() {_threadPool->wait();};
            void stop() { _stopRendering = true; };
            std::shared_ptr<Frame> getAvailableFrame();
            void setCurrentSceneSnapshot(std::shared_ptr<const RenderScene> scene) { this->_scene = std::move(scene); };
            std::vector<std::unique_ptr<IRayFeature>>& getPipeline() noexcept { return _pipelineManager->_pipeline; };

        private:
            void computeTile(const PixelTile &tile, const std::shared_ptr<Frame> &currentFrame, const RenderScene &scene, const Camera &cam);
            Graphic::Pixel getPixelColor(int x, int y, const RenderScene &scene, const Camera &cam, SurfaceIntersection &intersection, std::vector<Ray> &rayQueue);
            void executePipeline(const RenderScene &scene, SurfaceIntersection &intersection, ShaderType type);

            std::shared_ptr<const RenderScene> _scene; // current Scene snapshot.

            std::unique_ptr<optimization::ThreadPool> _threadPool;
            bool _stopRendering = false;

            std::unique_ptr<PipelineManager> _pipelineManager;

            std::queue<std::shared_ptr<Frame>> _readyPixelBuffers; // frame ready to be rendered by main thread.
            std::mutex _queueMutex;
            std::atomic<int> _framesInFlight;

            int _samplesPerPixel;

            // What the last frame was rendered from: while none of it changes, a still frame
            // is not rendered again since it would give the same image.
            struct FrameInputs {
                std::shared_ptr<const RenderScene> scene; // kept alive so a new snapshot can't reuse its address
                Math::Vector3D cameraPosition;
                Math::Vector3D cameraForward;
                double fov = 0.0;
                int width = 0;
                int height = 0;
                std::array<std::vector<IRayFeature *>, SHADER_TYPE_COUNT> features;

                bool operator==(const FrameInputs &) const = default;
            };
            FrameInputs _lastFrame;
            bool _lastFrameIsStill = false; // rendered from start to end with the still camera samples

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
