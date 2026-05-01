#include "integrators/rayFeature/SpecularHandler.hpp"

namespace RayTracer {
    static double calculateFresnel(double n1, double n2, double cosi)
    {
        double Fo = (n1 - n2) / (n1 + n2);
        Fo *= Fo;
        return Fo + (1.0 - Fo) * pow((1.0 - cosi), 5);
    }

    static void reflectRay(SurfaceIntersection &interaction, double cosi, Math::Vector3D N, double Re)
    {
        Math::Vector3D reflectionDir = (interaction.rayDir - N * (2.0f * cosi)).normalize();
        Math::Vector3D throughput = interaction.throughput * Re;
        interaction.nextRays.push_back({interaction.hitPoint + (N * 0.001f), reflectionDir, throughput, 0});
    }

    void SpecularHandler::process(RenderData &data, SurfaceIntersection &interaction)
    {
        Math::Vector3D N = interaction.normal;
        Math::Vector3D I = interaction.rayDir;

        float cosi = I.dot(N);

        double n1 = 1.0;
        double n2 = interaction.mat->ior;

        bool entering = cosi < 0.0;

        if (entering) {
            cosi = -cosi;
        } else {
            std::swap(n1, n2);
            N = N * -1;
        }

        float eta = (float)n1/n2;
        float k = 1.0 - (eta * eta) * (1.0 - (cosi * cosi));

        double F = calculateFresnel(n1, n2, cosi);
        double Re = F * interaction.mat->reflectivity; 
        double Ra = (1 - F) * interaction.mat->refractivity;

        if (k < 0.0 || Re >= 0.001) {
            // reflection
            reflectRay(interaction, cosi, N, Re);
        }
        if (k >= 0.0 && Ra >= 0.001){
            // refraction
            Math::Vector3D refractionDir = (I * eta) + (N * (eta * cosi - sqrt(k)));
            refractionDir = refractionDir.normalize();
            Math::Vector3D throughput = interaction.throughput * Ra;
            interaction.nextRays.push_back({interaction.hitPoint + (refractionDir* 0.001f), refractionDir, throughput, 0});
        }
        double diffuseMultiplier = std::max(0.0, 1.0 - (Ra + Re));
        interaction.localColor._r *= diffuseMultiplier;
        interaction.localColor._g *= diffuseMultiplier;
        interaction.localColor._b *= diffuseMultiplier;
    }
}
extern "C" RayTracer::IRayFeature *getInstance() {
    return new RayTracer::SpecularHandler();
}