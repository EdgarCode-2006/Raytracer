#include "integrators/rayFeature/SpecularHandler.hpp"

namespace RayTracer {
    static double calculateFresnel(double n1, double n2, double cosi)
    {
        double Fo = (n1 - n2) / (n1 + n2);
        Fo *= Fo;
        return Fo + (1.0 - Fo) * pow((1.0 - cosi), 5);
    }

    void SpecularHandler::process(RenderData &data, SurfaceIntersection &interaction)
    {
        Math::Vector3D N = interaction.normal;
        Math::Vector3D I = interaction.rayDir;

        float cosi = I.dot(N);

        double n1 = 1.0;
        double n2 = 1.8;
        bool entering = cosi < 0.0;

        if (entering) {
            cosi = -cosi;
        } else {
            std::swap(n1, n2);
            N = N * -1;
        }

        float eta = (float)n1/n2;
        float k = 1.0 - (eta * eta) * (1.0 - (cosi * cosi));
        
        interaction.shouldBounce = true;

        if (k < 0.0f) {
            // reflection
            Math::Vector3D reflectionDir = (I - N * (2.0f * cosi)).normalize();
            interaction.nextRay = Ray(interaction.hitPoint + (N * 0.001f), reflectionDir);
        } else {
            // refraction
            double FValue = calculateFresnel(n1, n2, cosi);
            Math::Vector3D refractionDir = (I * eta) + (N * (eta * cosi - sqrt(k)));
            refractionDir = refractionDir.normalize();
            interaction.nextRay = Ray(interaction.hitPoint - (N * 0.001f), refractionDir);
            interaction.throughput *= (1 - FValue);
        }
    }
}
extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::SpecularHandler();
}