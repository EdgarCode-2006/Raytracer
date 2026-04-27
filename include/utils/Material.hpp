#ifndef MATERIAL_HPP_
#define MATERIAL_HPP_
#include "utils/Pixel.hpp"

namespace Graphic {
    struct Material {
        Pixel color;
        float reflectivity;
        float refractivity;
        float ior; // index of refraction

        Material() : color(1.0, 1.0, 1.0, 0.0), reflectivity(0.0), refractivity(0.0), ior(1.0) {}
        Material(const Pixel& color, float reflectivity, float refractivity, float ior)
            : color(color), reflectivity(reflectivity), refractivity(refractivity), ior(ior) {}
    };
}

#endif