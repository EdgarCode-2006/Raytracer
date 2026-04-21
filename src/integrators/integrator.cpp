#include "integrators/Integrator.hpp"
#include "utils/math.hpp"
#include <raylib.h>

namespace RayTracer {
    Integrator::Integrator(const Scene& scene) : _scene(scene)
    {
        // Initialize integrator with scene
    }
    
    Integrator::~Integrator()
    {
        // Cleanup
    }
    
    void Integrator::computePixels(const Camera &cam, Color *pixels)
    {
        Math::Vector3D cameraDir = { 0.0f, 0.0f, 1.0f };
        Color col = {0, 0, 0};
        const int screenWidth = cam.getWidth();
        const int screenHeight = cam.getHeight();
    
        for (int y = 0; y < screenHeight; y++) {
            for (int x = 0; x < screenWidth; x++) {
    
                Ray ray = cam.generateRay(x, y);

                float value = ray.direction.dot(cameraDir);
                unsigned char intensity = (unsigned char)(value * 255);
                col = {intensity, intensity, intensity, 255};
    
                pixels[y * screenWidth + x] = col;
            }
        }
    }
}