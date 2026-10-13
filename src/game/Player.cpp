#include "game/Player.hpp"
#include <algorithm>

namespace Game {

    // how close to the edges of the screen the ship can go
    static constexpr double EDGE_MARGIN = 1.2;
    // how fast the ship rolls towards the angle it should have, per second
    static constexpr double BANK_RESPONSE = 10.0;

    void Player::reset(const PlayArea &area)
    {
        _transform.position = Math::Vector3D(area.left() + area.halfWidth * 0.4, area.centerY, area.depth);
        _transform.rotation = Math::Vector3D(0, 0, 0);
    }

    void Player::update(const Input::InputManager &input, double dt, const PlayArea &area)
    {
        const double ax = input.axisX();
        const double ay = -input.axisY(); // the input axis goes down like the screen, the world goes up
        Math::Vector3D direction(ax, ay, 0);
        if (ax != 0 && ay != 0)
            direction = direction.normalize(); // as fast in diagonal as straight

        Math::Vector3D &pos = _transform.position;
        pos += direction * (_definition->speed * dt);
        pos.x() = std::clamp(pos.x(), area.left() + EDGE_MARGIN, area.right() - EDGE_MARGIN);
        pos.y() = std::clamp(pos.y(), area.bottom() + EDGE_MARGIN * 0.6, area.top() - EDGE_MARGIN * 0.6);

        // roll around the length of the ship (x) when going up or down
        const double targetBank = ay * _definition->maxBank;
        double &bank = _transform.rotation.x();
        bank += (targetBank - bank) * std::min(1.0, BANK_RESPONSE * dt);
    }

    void Player::addToScene(RayTracer::Scene &scene) const
    {
        _definition->model.addTo(scene, _transform);
    }
}
