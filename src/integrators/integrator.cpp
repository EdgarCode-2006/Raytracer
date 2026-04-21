#include "integrators/Integrator.hpp"
#include "utils/math.hpp"
#include <raylib.h>
#include <cmath>
#include <future>

namespace RayTracer {
    Integrator::Integrator(const Scene& scene) : _scene(scene)
    {
        this->_threadPool = std::make_unique<optimization::ThreadPool>(4); // init thread pool with 4 threads.
        // Initialize integrator with scene
    }
    
    Integrator::~Integrator()
    {
        // Cleanup
    }

    void Integrator::computeTile(const PixelTile &tile, const Camera &cam, Color *pixels)
    {
        int startX = tile.getX();
        int startY = tile.getY();
        int width = tile.getXDim();
        int height = tile.getYDim();

        for (int y = startY; y < startY + height; y++) {
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
    
    void Integrator::renderScene(const Camera &cam, Color *pixels)
    {
        for (const auto& tile : _tiles) {
            this->_threadPool->enqueue([this, tile, &cam, pixels]() {
                this->computeTile(tile, cam, pixels);
            });
        }
    }

    Color Integrator::shootRay(const Ray &ray)
    {
        // SurfaceIntersection intersect = _scene.intersect(ray) // get from BVH
        Math::Vector3D cameraDir = { 0.0f, 0.0f, 1.0f };

        // using the intersection info we can move the ray further.
        // if (intersect.hit) {
        //     return calculateShading(intersect)
        // }
        float value = ray.direction.dot(cameraDir);
        unsigned char intensity = (unsigned char)(value * 255);
        Color col = {intensity, intensity, intensity, 255};
        return col;
    }

    void Integrator::waitForRendering()
    {
        _threadPool->wait();
    }
}
