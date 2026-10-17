#include "simulation/Enemy.hpp"
#include "common/World.hpp"
#include <cmath>

namespace Simulation {

    Enemy::Enemy(uint32_t id, const EnemyRules &rules, Common::Vec2 spawn)
        : _id(id), _rules(&rules), _position(spawn), _baseY(spawn.y)
    {
    }

    void Enemy::update(float dt)
    {
        _age += dt;
        const float phase = _rules->frequency * _age;
        _position.x -= _rules->speed * dt;
        _position.y = _baseY + _rules->amplitude * std::sin(phase);
        _velocity = {-_rules->speed, _rules->amplitude * _rules->frequency * std::cos(phase)};
    }

    bool Enemy::isPast() const noexcept
    {
        return _position.x < -Common::WORLD_HALF_WIDTH - 2.0f * _rules->size;
    }

    Common::EntityState Enemy::state() const noexcept
    {
        return {_id, _rules->type, _position, _velocity};
    }
}
