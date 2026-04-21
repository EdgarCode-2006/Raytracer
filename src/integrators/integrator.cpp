#include "integrators/Integrator.hpp"
#include "utils/math.hpp"
#include <raylib.h>

namespace RayTracer {
    Integrator::Integrator()
    {
        // load the functions from .so
    }
    
    Integrator::~Integrator()
    {
        // Close the .so
    }
    
    void Integrator::computePixels(const int &screenWidth, const int &screenHeight, Color *pixels)
    {
        Math::Vector3D cameraPos = { 0.0f, 0.0f, 0.0f };
        Math::Vector3D cameraDir = { 0.0f, 0.0f, 1.0f };
        float aspectRatio = (float)screenWidth / (float)screenHeight;
        Color col = {0, 0, 0};
        Camera cam = this->_scene->getCamera();
    
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