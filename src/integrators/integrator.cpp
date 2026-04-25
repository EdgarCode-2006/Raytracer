#include "integrators/Integrator.hpp"
#include "integrators/rayFeature/IRayFeature.hpp"
#include "utils/math.hpp"
#include <memory>
#include <chrono>

namespace RayTracer {
    Integrator::Integrator(const Scene& scene) : _scene(scene)
    {
        this->_threadPool = std::make_unique<optimization::ThreadPool>(10); // init thread pool with 4 threads.
        try {
            this->loadRayFeatures();
        } catch (const IntegratorException &e) {
            throw IntegratorException(e.what());
        }
        // Initialize integrator with scene

        const auto &tris = scene.getTriangles();
        if (!tris.empty()) {
            const auto start = std::chrono::high_resolution_clock::now();
            _bvh = std::make_unique<BVH>(tris);
            const auto end = std::chrono::high_resolution_clock::now();

            std::cout << "BVH build time: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";
        }
    }

    Integrator::~Integrator()
    {
        if (_threadPool) {
            _threadPool->wait();
            _threadPool.reset();
        }
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
                Ray ray = cam.generateRay(x, y);
                pixels[y * cam.getWidth() + x] = this->shootRay(ray);
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
