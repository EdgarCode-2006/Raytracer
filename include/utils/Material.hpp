#ifndef MATERIAL_HPP_
#define MATERIAL_HPP_
#include "utils/Pixel.hpp"

namespace Graphic {
    struct Material {
        Pixel color;
        float reflectivity;
        float refractivity;
        float ior; // index of refraction
        float emission;

        Material() :
            color(255, 255, 255, 255),
            reflectivity(0.0),
            refractivity(0.0),
            ior(1.0),
            emission(0.0) {}
        Material(const Pixel& color, float reflectivity, float refractivity, float ior, float emission)
            : color(color),
            reflectivity(reflectivity),
            refractivity(refractivity),
            ior(ior),
            emission(emission)
            {}
    };
}

#endif