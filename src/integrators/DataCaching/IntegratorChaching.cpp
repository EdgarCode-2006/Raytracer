#include "integrators/Integrator.hpp"

namespace RayTracer {
    void Integrator::instanceTiles(const Camera &cam)
    {
        this->_cache.tiles.clear();
        this->_framesInFlight = 0;
        {
            std::lock_guard<std::mutex> lock(this->_queueMutex);
            std::queue<std::shared_ptr<Frame>> empty;
            std::swap(_readyPixelBuffers, empty);
        }

        const int screenWidth = cam.getWidth();
        const int screenHeight = cam.getHeight();

        for (int y = 0; y < screenHeight; y += TILE_DIM) {
            for (int x = 0; x < screenWidth; x += TILE_DIM) {
                
                int currentWidth = std::min(TILE_DIM, screenWidth - x);
                int currentHeight = std::min(TILE_DIM, screenHeight - y);

                _cache.tiles.emplace_back(x, y, currentWidth, currentHeight);
            }
        }
    }
}
