#include "integrators/Integrator.hpp"

namespace RayTracer {
    class Frame {
    public:
        ~Frame() = default;
        Frame(int bufferSize, int tileCount) 
            : _tileCount(tileCount), _completedTileCount(0) {
            _pixelBuffer.resize(bufferSize);
        }

        bool IncrementAndCheckCompletion() {
            return (++_completedTileCount == _tileCount);
        }

        std::vector<Graphic::Pixel> _pixelBuffer;

    private:
        const int _tileCount;
        std::atomic<int> _completedTileCount;
    };
}