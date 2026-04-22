#include "integrators/Integrator.hpp"
#include "utils/math.hpp"
#include <raylib.h>

namespace RayTracer {
    Integrator::Integrator(const Scene& scene) : _scene(scene)
    {
        this->_threadPool = std::make_unique<optimization::ThreadPool>(10); // init thread pool with 4 threads.
        // Initialize integrator with scene
    }
    
    Integrator::~Integrator()
    {
        // Cleanup
    }

    void Integrator::computeTile(const PixelTile &tile, const Camera &cam, Graphic::Pixel *pixels)
    {
        int startX = tile.getX();
        int startY = tile.getY();
        int width = tile.getXDim();
        int height = tile.getYDim();

        for (int y = startY; y < startY + height; y++) {
            if (_stopRender) return;
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

    double Integrator::intersectTriangle(const Ray& ray, const Triangle& tri)
    {
        const double EPSILON = 1e-7;
        Math::Vector3D edge1 = tri.b() - tri.a();
        Math::Vector3D edge2 = tri.c() - tri.a();
        Math::Vector3D h = ray.direction.cross(edge2);
        double a = edge1.dot(h);
        if (a > -EPSILON && a < EPSILON)
            return -1;
        double f = 1.0 / a;
        Math::Vector3D s = ray.origin - tri.a();
        double u = f * s.dot(h);
        if (u < 0.0 || u > 1.0)
            return -1;
        Math::Vector3D q = s.cross(edge1);
        double v = f * ray.direction.dot(q);
        if (v < 0.0 || u + v > 1.0)
            return -1;
        double t = f * edge2.dot(q);
        return (t > EPSILON) ? t : -1;
    }

    Graphic::Pixel Integrator::shootRay(const Ray &ray)
    {
        double closest_t = 1e9;
        const Triangle* hit = nullptr;

        for (const auto& tri : this->_scene.getTriangles()) {
            double t = intersectTriangle(ray, tri);
            if (t > 0 && t < closest_t) {
                closest_t = t;
                hit = &tri;
            }
        }

        if (!hit)
            return {20, 20, 20, 255};

        Math::Vector3D normal = (hit->b() - hit->a()).cross(hit->c() - hit->a()).normalize();

        double intensity = this->_scene.getAmbientLight().intensity;
        for (const auto& light : this->_scene.getLights()) {
            double diff = normal.dot(light.direction * -1.0);
            if (diff > 0)
                intensity += diff * light.intensity;
        }
        if (intensity > 1.0)
            intensity = 1.0;

        unsigned char c = (unsigned char)(intensity * 255);
        return {c, c, c, 255};
    }

    void Integrator::waitForRendering()
    {
        _threadPool->wait();
    }
}
