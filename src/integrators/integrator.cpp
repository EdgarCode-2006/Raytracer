#include "integrators/Integrator.hpp"
#include "integrators/rayFeature/IRayFeature.hpp"
#include "opti/BVH/Instance.hpp"
#include "opti/BVH/TLAS.hpp"
#include "opti/BVH/TLASHit.hpp"
#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include <utility>

namespace RayTracer {
    Integrator::Integrator(const Scene &scene) : _scene(scene)
    {
        this->_threadPool = std::make_unique<optimization::ThreadPool>(15); // init thread pool with 4 threads.
        try {
            this->loadRayFeatures();
        } catch (const IntegratorException &e) {
            throw IntegratorException(e.what());
        }

        _samplesPerSide = std::sqrt(RAYS_PER_PIXEL);
        if (_samplesPerSide < 1) _samplesPerSide = 1;

        _step = 1.0f / _samplesPerSide;
    }

    Integrator::~Integrator()
    {
        if (_threadPool) {
            _threadPool->wait();
            _threadPool.reset();
        }
    }

    void Integrator::getPixelColor(int x, int y, const Camera &cam, Graphic::Pixel *pixels)
    {
        float sumR = 0, sumG = 0, sumB = 0;

        for (int sY = 0; sY < _samplesPerSide; sY++) {
            for (int sX = 0; sX < _samplesPerSide; sX++) {
                
                float offsetX = (sX + 0.5f) * _step - 0.5f;
                float offsetY = (sY + 0.5f) * _step - 0.5f;

                Ray ray = cam.generateRay(x + offsetX, y + offsetY);
                Graphic::Pixel sampleColor = this->shootRay(ray);
                
                sumR += sampleColor._r;
                sumG += sampleColor._g;
                sumB += sampleColor._b;
            }
        }

        int totalSamples = _samplesPerSide * _samplesPerSide;
        
        pixels[y * cam.getWidth() + x] = Graphic::Pixel(
            (unsigned char)(sumR / totalSamples),
            (unsigned char)(sumG / totalSamples),
            (unsigned char)(sumB / totalSamples));
    }

    void Integrator::computeTile(const PixelTile &tile, const Camera &cam, Graphic::Pixel *pixels)
    {
        int startX = tile.getX();
        int startY = tile.getY();
        int width = tile.getXDim();
        int height = tile.getYDim();

        for (int y = startY; y < startY + height; y++) {
            if (_stopRendering) return;
            for (int x = startX; x < startX + width; x++) {
                this->getPixelColor(x, y, cam, pixels);
            }
        }
    }

    void Integrator::instanceTiles(const Camera &cam)
    {
        this->_tiles.clear();
    
        const int screenWidth = cam.getWidth();
        const int screenHeight = cam.getHeight();

        for (int y = 0; y < screenHeight; y += TILE_DIM) {
            for (int x = 0; x < screenWidth; x += TILE_DIM) {
                
                int currentWidth = std::min(TILE_DIM, screenWidth - x);
                int currentHeight = std::min(TILE_DIM, screenHeight - y);

                _tiles.emplace_back(x, y, currentWidth, currentHeight);
            }
        }
    }
    
    void Integrator::renderScene(const Camera &cam, Graphic::Pixel* pixels)
    {
        for (const auto& tile : _tiles) {
            this->_threadPool->enqueue([this, tile, &cam, pixels]() {
                this->computeTile(tile, cam, pixels);
            });
        }
    }

    void Integrator::waitForRendering()
    {
        _threadPool->wait();
    }

    void Integrator::setPluginState(const std::string &pluginName, const bool &state)
    {
        for (auto& plugin : _pipeline) {
            if (plugin->getName() == pluginName) {
                plugin->setActivity(state);
                return;
            }
        }
        std::cout << "plugin not found couldn't disable." << std::endl;
    }
}
