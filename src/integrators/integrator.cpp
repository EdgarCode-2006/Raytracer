#include "integrators/Integrator.hpp"
#include "utils/math.hpp"
#include <raylib.h>

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

    for (int y = 0; y < screenHeight; y++) {
        for (int x = 0; x < screenWidth; x++) {

            float screenX = ((2.0f * (x + 0.5f) / screenWidth) - 1.0f) * aspectRatio;
            float screenY = 1.0f - (2.0f * (y + 0.5f) / screenHeight);

            Math::Vector3D rayDir = (Math::Vector3D(screenX, screenY, 1.0f)).normalize();
            float value = rayDir.dot(cameraDir);
            unsigned char intensity = (unsigned char)(value * 255);
            col = {intensity, intensity, intensity, 255};

            pixels[y * screenWidth + x] = col;
        }
    }
}