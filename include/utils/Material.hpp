#ifndef MATERIAL_HPP_
#define MATERIAL_HPP_
#include "utils/Pixel.hpp"
#include "entities/Texture.hpp"
#include <memory>

enum ShaderType {
    EMISSION  = 1 << 0,
    DIFFUSION = 1 << 1,
    CAUSTIC   = 1 << 2,
    UTILITY   = 1 << 3,
    DEBUG     = 1 << 4
};

namespace Graphic {
    struct Material {
        Pixel color;
        float reflectivity;
        float refractivity;
        float ior; // index of refraction
        float emission;
        ShaderType type;
        std::shared_ptr<RayTracer::Texture> texture = nullptr;
        bool useTexture = false;

        Material() :
            color(255, 255, 255, 255),
            reflectivity(0.0),
            refractivity(0.0),
            ior(1.0),
            emission(0.0),
            type(DIFFUSION)
            {}
        Material(const Pixel& color, float reflectivity,
            float refractivity, float ior, float emission, const ShaderType &t)
            : color(color),
            reflectivity(reflectivity),
            refractivity(refractivity),
            ior(ior),
            emission(emission),
            type(t)
            {}
    };
}

#endif
