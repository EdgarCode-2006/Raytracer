#ifndef PLAY_AREA_HPP_
#define PLAY_AREA_HPP_

#include "entities/Camera.hpp"
#include <cmath>
#include <numbers>

namespace Game {

    // The rectangle the camera sees on the plane the game is played on, in world units.
    // The camera looks along +z: x goes to the right of the screen and y up.
    struct PlayArea {
        double centerX = 0.0;
        double centerY = 0.0;
        double depth = 0.0; // z of the plane
        double halfWidth = 0.0;
        double halfHeight = 0.0;

        static PlayArea fromCamera(const RayTracer::Camera &cam, double distance) {
            PlayArea area;
            const Math::Vector3D &pos = cam.getPosition();
            area.centerX = pos.x();
            area.centerY = pos.y();
            area.depth = pos.z() + distance;
            area.halfHeight = distance * std::tan(cam.getFov() / 2.0 * std::numbers::pi / 180.0);
            area.halfWidth = area.halfHeight * cam.getWidth() / cam.getHeight();
            return area;
        }

        double left() const noexcept { return centerX - halfWidth; }
        double right() const noexcept { return centerX + halfWidth; }
        double bottom() const noexcept { return centerY - halfHeight; }
        double top() const noexcept { return centerY + halfHeight; }
    };
}

#endif
