#include "client/SceneBuilder.hpp"
#include <algorithm>
#include <cmath>

namespace Client {

    // how fast a ship rolls towards the angle it should have, per second
    static constexpr double BANK_RESPONSE = 10.0;

    SceneBuilder::SceneBuilder(RayTracer::Scene &scene, const ModelLibrary &models, const ClientSettings &settings)
        : _scene(scene), _models(models), _settings(settings), _firstInstance(scene.getInstanceCount())
    {
    }

    void SceneBuilder::apply(const Common::WorldState &world, double dt)
    {
        _applyCount++;
        _scene.removeInstancesFrom(_firstInstance);

        for (const auto &entity : world.entities) {
            const Model &model = _models.get(entity.type);
            Animation &anim = _animations[entity.id];
            anim.lastSeen = _applyCount;

            const double vertical = std::clamp(entity.velocity.y / model.bankAtSpeed, -1.0, 1.0);
            anim.bank += (vertical * model.maxBank - anim.bank) * std::min(1.0, BANK_RESPONSE * dt);
            anim.spin = std::fmod(anim.spin + model.spin * dt, 360.0);

            Transform transform;
            transform.position = Math::Vector3D(entity.position.x, entity.position.y, 0.0);
            transform.rotation = Math::Vector3D(
                anim.bank + (model.spinAxis == 0 ? anim.spin : 0.0),
                model.spinAxis == 1 ? anim.spin : 0.0,
                model.spinAxis == 2 ? anim.spin : 0.0);
            model.addTo(_scene, transform, _settings.highContrast, entity.variant);
        }

        // forget the entities the server doesn't send anymore
        std::erase_if(_animations, [this](const auto &entry) { return entry.second.lastSeen != _applyCount; });
    }

    void SceneBuilder::clear()
    {
        _scene.removeInstancesFrom(_firstInstance);
        _animations.clear();
    }
}
