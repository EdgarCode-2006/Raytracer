#include "simulation/Player.hpp"
#include "common/World.hpp"
#include <algorithm>

namespace Simulation {

    Player::Player(uint32_t id, const PlayerRules &rules, Common::Vec2 start)
        : _id(id), _rules(&rules), _position(start)
    {
    }

    void Player::update(float dt)
    {
        // as fast in diagonal as straight
        _velocity = _input.direction().normalized() * _rules->speed;
        _position += _velocity * dt;

        const float maxX = Common::WORLD_HALF_WIDTH - _rules->size;
        const float maxY = Common::WORLD_HALF_HEIGHT - _rules->size * 0.6f;
        _position.x = std::clamp(_position.x, -maxX, maxX);
        _position.y = std::clamp(_position.y, -maxY, maxY);
    }

    Common::EntityState Player::state() const noexcept
    {
        return {_id, Common::EntityType::Player, _position, _velocity};
    }
}
