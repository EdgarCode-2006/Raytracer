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
        this->_threadPool = std::make_unique<optimization::ThreadPool>(10); // init thread pool with 4 threads.
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

    Graphic::Pixel Integrator::getPixelColor(std::tuple<int, int> cords,
        std::shared_ptr<const RenderScene> sceneLocked,
        const Camera &cam)
    {
        auto [x, y] = cords;
        float sumR = 0, sumG = 0, sumB = 0;

        int samplesPerSide = std::sqrt(_samplesPerPixel);
        if (samplesPerSide < 1) samplesPerSide = 1;

        float step = 1.0f / samplesPerSide;

        for (int sY = 0; sY < samplesPerSide; sY++) {
            for (int sX = 0; sX < samplesPerSide; sX++) {
                
                float offsetX = (sX + 0.5f) * step - 0.5f;
                float offsetY = (sY + 0.5f) * step - 0.5f;

                Ray ray = cam.generateRay(x + offsetX, y + offsetY);
                Graphic::Pixel sampleColor = this->shootRay(ray, sceneLocked);
                
                sumR += sampleColor._r;
                sumG += sampleColor._g;
                sumB += sampleColor._b;
            }
        }

        int totalSamples = samplesPerSide * samplesPerSide;

        return Graphic::Pixel(
            (unsigned char)(sumR / totalSamples),
            (unsigned char)(sumG / totalSamples),
            (unsigned char)(sumB / totalSamples));
    }

    void Integrator::computeTile(const PixelTile &tile,
        std::shared_ptr<Frame> currentFrame,
        std::shared_ptr<const RenderScene> sceneLocked,
        const Camera &cam)
    {
        int startX = tile.getX();
        int startY = tile.getY();
        int width = tile.getXDim();
        int height = tile.getYDim();

        for (int y = startY; y < startY + height; y++) {
            if (_stopRendering) return;
            for (int x = startX; x < startX + width; x++) {
                Graphic::Pixel pColor = getPixelColor(std::tuple<int, int>(x, y), sceneLocked, cam);
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
        this->_samplesPerPixel = this->_cache.prevCamPos == cam.getPosition() ? DEFAULT_SAMPLES_PER_PIXEL : 1;
        this->_cache.prevCamPos = cam.getPosition();

        if (this->_framesInFlight >= 1)
            return;

        std::shared_ptr<const RenderScene> sceneLocked = this->_scene;\

        _framesInFlight += 1;

        int bufferSize = cam.getWidth() * cam.getHeight();
        int tileCount = static_cast<int>(_cache.tiles.size());
        auto currentFrame = std::make_shared<Frame>(bufferSize, tileCount);

        for (const auto& tile : _cache.tiles) {
            this->_threadPool->enqueue([this, tile, currentFrame, sceneLocked, cam]() {
                this->computeTile(tile, currentFrame, sceneLocked, cam); 
            });
        }
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
