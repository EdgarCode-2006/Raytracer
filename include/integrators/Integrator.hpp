#ifndef INTEGRATOR_HPP_
#define INTEGRATOR_HPP_
#include <memory>
#include "Scene.hpp"
#include <raylib.h>
#include "integrators/PixelTile.hpp"
#include "opti/ThreadPool/threadPool.hpp"

#define TILE_DIM 32

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
            void renderScene(const Camera &cam, Color *pixels);
            void instanceTiles(const Camera &cam);
            Color shootRay(const Ray &ray);
            void waitForRendering();

        private:
            void computeTile(const PixelTile &tile, const Camera &cam, Color *pixels);
            double intersectTriangle(const Ray& ray, const Triangle& tri);
            const Scene& _scene;
            std::unique_ptr<optimization::ThreadPool> _threadPool;
            std::vector<PixelTile> _tiles;
    };
}
#endif
