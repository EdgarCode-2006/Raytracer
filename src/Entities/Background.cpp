#include "entities/Background.hpp"
#include <algorithm>
#include <stdexcept>

namespace RayTracer {

    void Background::addLayer(const std::string &texturePath, double speed)
    {
        auto texture = std::make_shared<Texture>(texturePath);
        if (!texture->isLoaded())
            throw std::runtime_error("Cannot load background layer: " + texturePath);
        _layers.push_back({std::move(texture), speed});
    }

    Graphic::Pixel Background::sample(const Math::Vector3D &direction, double time) const
    {
        const double x = direction.x() / direction.z();
        const double y = direction.y() / direction.z();
        int r = 0, g = 0, b = 0;

        for (const auto &layer : _layers) {
            const double aspect = (double)layer.texture->getWidth() / layer.texture->getHeight();
            const double u = (x / _viewHeight + layer.speed * time) / aspect;
            const double v = 0.5 - y / _viewHeight;
            const Graphic::Pixel texel = layer.texture->sampleWrapped(u, v);
            r += texel._r;
            g += texel._g;
            b += texel._b;
        }
        return Graphic::Pixel(
            (unsigned char)std::min(r, 255),
            (unsigned char)std::min(g, 255),
            (unsigned char)std::min(b, 255),
            255);
    }
}
