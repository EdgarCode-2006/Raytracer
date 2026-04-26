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
#include <random>

#include "Scene.hpp"
#include <raylib.h>
#include "integrators/PixelTile.hpp"
#include "opti/BVH/BVH.hpp"
#include "opti/ThreadPool/threadPool.hpp"
#include "utils/Pixel.hpp"
#include "entities/Light.hpp"
#include "integrators/rayFeature/IRayFeature.hpp"
#include "integrators/featureLoader/DLLoader.hpp"
#include "utils/Material.hpp"

#define TILE_DIM 64
#define MIN_THROUGHPUT_LENGTH 0.001
#define MAX_DEPTH 5
#define RAYS_PER_PIXEL 1

namespace RayTracer {
    struct SurfaceIntersection
    {
        Math::Vector3D normal;
        Math::Vector3D hitPoint;
        Math::Vector3D rayDir;
        // Graphic::Material *mat;
        Graphic::Pixel localColor;
        RayTracer::Ray nextRay;
        bool shouldBounce;
        Math::Vector3D throughput;

        Math::Vector3D getOmega() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<double> dist(0.0, 1.0);

            double r1 = dist(gen);
            double r2 = dist(gen);

            double r = std::sqrt(r1);
            double phi = 2 * M_PI * r2;

            double x = r * std::cos(phi);
            double y = r * std::sin(phi);
            double z = std::sqrt(1 - r1);

            // Build coordinate system from normal
            Math::Vector3D n = normal;
            Math::Vector3D ref = (std::abs(n.z()) < 0.999) ? Math::Vector3D(0, 0, 1) : Math::Vector3D(1, 0, 0);
            Math::Vector3D tangent = n.cross(ref).normalize();
            Math::Vector3D bitangent = n.cross(tangent);

            return (tangent * x + bitangent * y + n * z).normalize();
        }
    };
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
