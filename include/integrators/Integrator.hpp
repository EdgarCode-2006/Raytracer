#ifndef INTEGRATOR_HPP_
#define INTEGRATOR_HPP_
#include <memory>
#include "Scene.hpp"
#include <raylib.h>
#include "integrators/PixelTile.hpp"
#include "opti/BVH/BVH.hpp"
#include "opti/ThreadPool/threadPool.hpp"
#include "utils/Pixel.hpp"

#define TILE_DIM 64

namespace RayTracer {
    struct SurfaceIntersection
    {
        Math::Vector3D normal;
        Math::Vector3D hitPoint;
        float t;
        // Material *mat
        bool hit;
    };
    class Integrator {
        public:
            Integrator(const Scene& scene);
            ~Integrator();
            void renderScene(const Camera &cam, Graphic::Pixel *pixels);
            void instanceTiles(const Camera &cam);
            Graphic::Pixel shootRay(const Ray &ray);
            void waitForRendering();
            void stop() { _stopRendering = true; };
            
        private:
            double intersectTriangle(const Ray& ray, const Triangle& tri);
            void computeTile(const PixelTile &tile, const Camera &cam, Graphic::Pixel *pixels);
            const Scene& _scene;
            std::unique_ptr<optimization::ThreadPool> _threadPool;
            std::vector<PixelTile> _tiles;
            bool _stopRendering = false;
            std::unique_ptr<BVH> _bvh;
    };
}
#endif
