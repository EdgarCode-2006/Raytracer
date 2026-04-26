#ifndef INTEGRATOR_HPP_
#define INTEGRATOR_HPP_
#include <memory>
#include <string>
#include <vector>
#include <poll.h>
#include <netinet/in.h>
#include <unordered_map>
#include <memory>
#include <cmath>

#include "Scene.hpp"
#include <raylib.h>
#include "integrators/PixelTile.hpp"
#include "opti/BVH/BVH.hpp"
#include "opti/ThreadPool/threadPool.hpp"
#include "utils/Pixel.hpp"
#include "utils/surfaceIntersection.hpp"
#include "entities/Light.hpp"
#include "integrators/rayFeature/IRayFeature.hpp"
#include "integrators/featureLoader/DLLoader.hpp"

#define TILE_DIM 64
#define MIN_THROUGHPUT_LENGTH 0.001
#define MAX_DEPTH 5
#define RAYS_PER_PIXEL 1

namespace RayTracer {
    struct RenderData
    {
        std::vector<DirectionalLight> lights;
        const AmbientLight& ambientLight;
        const Scene& scene;
    };

    class IRayFeature;

    class Integrator {
        public:
            Integrator(const Scene& scene);
            ~Integrator();
            void renderScene(const Camera &cam, Graphic::Pixel *pixels);
            void instanceTiles(const Camera &cam);
            Graphic::Pixel shootRay(Ray &ray);
            void waitForRendering();
            void stop() { _stopRendering = true; };
            void loadRayFeatures();
            
        private:
            void computeTile(const PixelTile &tile, const Camera &cam, Graphic::Pixel *pixels);
            void setPluginState(const std::string &pluginName, const bool &state);
            void getPixelColor(int x, int y, const Camera &cam, Graphic::Pixel *pixels);
            const Scene& _scene;
            std::unique_ptr<optimization::ThreadPool> _threadPool;
            std::vector<PixelTile> _tiles;
            bool _stopRendering = false;

            std::vector<DLLoader<IRayFeature>> _loaders;
            std::vector<std::unique_ptr<IRayFeature>> _pipeline;
            std::unique_ptr<BVH> _bvh;

            int _samplesPerSide;
            float _step;
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
