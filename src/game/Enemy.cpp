#include "game/Enemy.hpp"
#include <cmath>

namespace Game {

    Enemy::Enemy(const EnemyDefinition &definition, const Math::Vector3D &spawn)
        : _definition(&definition), _baseY(spawn.y())
    {
        _transform.position = spawn;
    }

    void Enemy::update(double dt)
    {
        _age += dt;
        Math::Vector3D &pos = _transform.position;
        pos.x() -= _definition->speed * dt;
        pos.y() = _baseY + _definition->amplitude * std::sin(_definition->frequency * _age);
        const double angle = std::fmod(_definition->spin * _age, 360.0);
        _transform.rotation = Math::Vector3D(
            _definition->spinAxis == 0 ? angle : 0.0,
            _definition->spinAxis == 1 ? angle : 0.0,
            _definition->spinAxis == 2 ? angle : 0.0);
    }

    bool Enemy::isPast(const PlayArea &area) const noexcept
    {
        // far enough for the whole model to be out of the screen
        return _transform.position.x() < area.left() - 2.0 * _definition->model.scale;
    }

    void Enemy::addToScene(RayTracer::Scene &scene) const
    {
        _definition->model.addTo(scene, _transform);
    }
}
