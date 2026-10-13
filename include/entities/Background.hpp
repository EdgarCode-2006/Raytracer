#ifndef BACKGROUND_HPP_
#define BACKGROUND_HPP_

#include "entities/Texture.hpp"
#include "utils/Pixel.hpp"
#include "utils/math.hpp"
#include <memory>
#include <string>
#include <vector>

namespace RayTracer {

    // What the rays that hit nothing see in front of the camera: tiling textures added on top of
    // each other, each scrolling to the left at its own speed (parallax).
    // The layers are on a plane far away facing the camera, which looks along +z: a direction maps
    // to (x / z, y / z), so the textures stay flat on screen instead of bending like an HDRI.
    class Background {
        public:
            struct Layer {
                std::shared_ptr<const Texture> texture;
                double speed; // in view heights per second
            };

            void addLayer(const std::string &texturePath, double speed);
            void clearLayers() noexcept { _layers.clear(); }
            bool isEmpty() const noexcept { return _layers.empty(); }

            // how much of the (x / z, y / z) plane one texture height covers: 2 * tan(fov / 2) fills the view
            void setViewHeight(double height) noexcept { _viewHeight = height; }

            // false when the direction doesn't go towards the background plane
            bool covers(const Math::Vector3D &direction) const noexcept { return !isEmpty() && direction.z() > 1e-6; }
            // time is the scrolling time in seconds: it is kept by the scene snapshot
            Graphic::Pixel sample(const Math::Vector3D &direction, double time) const;

        private:
            std::vector<Layer> _layers;
            double _viewHeight = 1.1547; // 2 * tan(30 degrees), for the default 60 degrees fov
    };
}

#endif
