#include "integrators/Integrator.hpp"
#include "integrators/rayFeature/IRayFeature.hpp"
#include "utils/math.hpp"
#include <memory>

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

        // const auto &tris = scene.getAllTriangles();
        // if (!tris.empty()) {
        //     const auto start = std::chrono::high_resolution_clock::now();
        //     _bvh = std::make_unique<BVH>(tris);
        //     const auto end = std::chrono::high_resolution_clock::now();

        //     std::cout << "BVH build time: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";
        // }
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

    Graphic::Pixel Integrator::shootRay(Ray &ray)
    {
        float energy = 1.0;
        RenderData data{
            .lights = this->_scene.getLights(),
            .ambientLight = this->_scene.getAmbientLight(),
            .scene = this->_scene,
        };
        SurfaceIntersection intersection{
            .normal = {0, 0, 0},
            .hitPoint = {0, 0, 0},
            .rayDir = ray.direction,
            .accumulatedColor = {0, 0, 0, 255},
            .nextRay = ray,
            .shouldBounce = false,
            .attenuation = 1.0f
        };

        for (size_t i = 0; i < MAX_DEPTH; i++) {
            double closest_t = 1e9;
            const Triangle* hit = nullptr;

            for (const auto& tri : data.scene.getAllTriangles()) {
                double t = intersectTriangle(ray, tri);
                if (t > 0 && t < closest_t) {
                    closest_t = t;
                    hit = &tri;
                }
            }
            if (!hit)
                break;
            intersection.hitPoint = ray.origin + ray.direction * closest_t;
            intersection.normal = (hit->b() - hit->a()).cross(hit->c() - hit->a()).normalize();
            intersection.rayDir = ray.direction;
            for (auto &rayFeature : _pipeline) {
                rayFeature->process(data, intersection);
            }
            if (intersection.shouldBounce) {
                energy /= intersection.attenuation + 0.00001f;
                ray = intersection.nextRay;
            } else {
                break;
            }
        }
        return intersection.accumulatedColor;
    }

    void Integrator::waitForRendering()
    {
        _threadPool->wait();
    }
}
