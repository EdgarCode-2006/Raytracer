#ifndef MATERIAL_HPP_
#define MATERIAL_HPP_
#include "utils/Pixel.hpp"
#include "entities/Texture.hpp"
#include <memory>

enum ShaderType {
    EMISSION,
    DIFFUSION,
    CAUSTIC,
    UTILITY,
    DEBUG,
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