#include "integrators/Integrator.hpp"
#include "integrators/rayFeature/IRayFeature.hpp"
#include "opti/BVH/Instance.hpp"
#include "opti/BVH/TLAS.hpp"
#include "opti/BVH/TLASHit.hpp"
#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include <utility>

namespace RayTracer {
    Integrator::Integrator(std::shared_ptr<const RenderScene> scene) : _scene(std::move(scene))
    {
        // one render thread per core (hardware_concurrency() returns 0 when it can't tell)
        const unsigned int cores = std::thread::hardware_concurrency();
        this->_threadPool = std::make_unique<optimization::ThreadPool>(cores > 0 ? cores : 4);
        this->_cache = {};
        try
        {
           this->_pipelineManager = std::make_unique<PipelineManager>();
        } catch(const IntegratorException &e)
        {
            throw IntegratorException(e.what());
        }
        
        _samplesPerPixel = 8;
        this->_cache.prevCamPos = {0, 0, 0};
    }

    Integrator::~Integrator()
    {
        if (_threadPool) {
            _threadPool->wait();
            _threadPool.reset();
        }
    }

    Graphic::Pixel Integrator::getPixelColor(int x, int y, const RenderScene &scene,
        const Camera &cam, SurfaceIntersection &intersection, std::vector<Ray> &rayQueue)
    {
        const int samplesPerPixel = _samplesPerPixel;

        // Default for no-hit = 1 sample for HDRI
        int numSamples = 1;

        // the probe ray only decides between 1 and several samples
        if (samplesPerPixel > 1 && scene.tlas->traverse(cam.generateRay(x, y)).hasHit()) {
            numSamples = DEFAULT_SAMPLES_PER_PIXEL;
        }

        numSamples = std::clamp(numSamples, 1, samplesPerPixel);

        float sumR = 0, sumG = 0, sumB = 0;
        int samplesPerSide = std::max(1, (int)std::sqrt(numSamples));
        int totalSamples = samplesPerSide * samplesPerSide;
        float step = 1.0f / samplesPerSide;

        for (int sY = 0; sY < samplesPerSide; sY++) {
            for (int sX = 0; sX < samplesPerSide; sX++) {
                float offsetX = (sX + 0.5f) * step - 0.5f;
                float offsetY = (sY + 0.5f) * step - 0.5f;

                Ray ray = cam.generateRay(x + offsetX, y + offsetY);
                // the hit specific fields are set by shootRay before any feature reads them
                intersection.rayDir = ray.direction;
                intersection.localColor = {0, 0, 0, 255};
                intersection.throughput = {1.0, 1.0, 1.0};
                intersection.nextRays.clear();
                intersection.nextRays.push_back(ray);

                Graphic::Pixel sampleColor = this->shootRay(intersection, scene, rayQueue);
                sumR += sampleColor._r;
                sumG += sampleColor._g;
                sumB += sampleColor._b;
            }
        }

        return Graphic::Pixel(
            (unsigned char)(sumR / totalSamples),
            (unsigned char)(sumG / totalSamples),
            (unsigned char)(sumB / totalSamples));
    }

    void Integrator::computeTile(const PixelTile &tile,
        const std::shared_ptr<Frame> &currentFrame,
        const RenderScene &scene,
        const Camera &cam)
    {
        int startX = tile.getX();
        int startY = tile.getY();
        int width = tile.getXDim();
        int height = tile.getYDim();
        // reused by every sample of the tile, so that their ray lists are allocated once
        SurfaceIntersection intersection{};
        std::vector<Ray> rayQueue;

        for (int y = startY; y < startY + height; y++) {
            if (_stopRendering) return;
            for (int x = startX; x < startX + width; x++) {
                Graphic::Pixel pColor = getPixelColor(x, y, scene, cam, intersection, rayQueue);
                currentFrame->_pixelBuffer[y * cam.getWidth() + x] = pColor;
            }
        }
        if (currentFrame->IncrementAndCheckCompletion()) {
            {
                std::lock_guard<std::mutex> lock(this->_queueMutex);
                _readyPixelBuffers.push(currentFrame);
            }
            this->_framesInFlight--;
        }
    }

    void Integrator::renderScene(const Camera &cam)
    {
        // still: neither the camera nor the scene moved since the last call. With a fixed camera
        // the game still changes the scene every frame, which must not take many samples either.
        const bool still = this->_cache.prevCamPos == cam.getPosition() && this->_cache.prevCamForward == cam.getForward()
            && this->_cache.prevScene == this->_scene;
        this->_samplesPerPixel = still ? DEFAULT_SAMPLES_PER_PIXEL : 1;
        this->_cache.prevCamPos = cam.getPosition();
        this->_cache.prevCamForward = cam.getForward();
        this->_cache.prevScene = this->_scene;

        if (this->_framesInFlight >= 1) {
            // the tiles left in the frame being rendered now take a single sample
            if (!still)
                _lastFrameIsStill = false;
            return;
        }

        std::shared_ptr<const RenderScene> sceneLocked = this->_scene;
        this->_pipelineManager->refreshActiveFeatures();

        FrameInputs inputs{sceneLocked, cam.getPosition(), cam.getForward(), cam.getFov(),
            cam.getWidth(), cam.getHeight(), this->_pipelineManager->getActiveFeatures()};
        if (still && _lastFrameIsStill && inputs == _lastFrame)
            return; // the frame on screen already shows exactly this
        _lastFrame = std::move(inputs);
        _lastFrameIsStill = still;

        _framesInFlight += 1;

        int bufferSize = cam.getWidth() * cam.getHeight();
        int tileCount = static_cast<int>(_cache.tiles.size());
        auto currentFrame = std::make_shared<Frame>(bufferSize, tileCount);

        std::vector<std::function<void()>> tasks;
        tasks.reserve(_cache.tiles.size());
        for (const auto& tile : _cache.tiles) {
            tasks.emplace_back([this, tile, currentFrame, sceneLocked, cam]() {
                this->computeTile(tile, currentFrame, *sceneLocked, cam);
            });
        }
        this->_threadPool->enqueueBatch(std::move(tasks));
    }

    std::shared_ptr<Frame> Integrator::getAvailableFrame()
    {
        std::shared_ptr<Frame> newFrame = nullptr;
        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            if (!_readyPixelBuffers.empty()) {
                newFrame = _readyPixelBuffers.front();
                _readyPixelBuffers.pop();
            }
        }
        return newFrame;
    }
}
