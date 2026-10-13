#ifndef GAME_ASSETS_HPP_
#define GAME_ASSETS_HPP_

#include "core/Scene.hpp"
#include "utils/Material.hpp"
#include "utils/math.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace Game {

    // Where an entity is: rotation in degrees around x, y and z.
    struct Transform {
        Math::Vector3D position;
        Math::Vector3D rotation;
    };

    // One mesh of a model with its own material (a hull and a glowing engine for example).
    struct ModelPart {
        uint32_t meshId;
        Graphic::Material material;
    };

    struct Model {
        std::vector<ModelPart> parts;
        double scale = 1.0;

        // adds one instance per part, all at the same transform
        void addTo(RayTracer::Scene &scene, const Transform &transform) const;
    };

    struct PlayerDefinition {
        Model model;
        double speed = 9.0;    // world units per second
        double maxBank = 25.0; // degrees of roll when moving up or down
    };

    struct EnemyDefinition {
        std::string name;
        Model model;
        double speed = 4.0;     // to the left, world units per second
        double amplitude = 0.0; // of the vertical wave, world units
        double frequency = 0.0; // of the vertical wave, radians per second
        double spin = 0.0;      // degrees per second
        int spinAxis = 1;       // 0 for x, 1 for y, 2 for z (z spins in the plane of the screen)
    };

    // Everything the game spawns, read from assets/game/entities.cfg. The meshes are loaded into
    // the scene once at startup, before anything renders.
    struct GameAssets {
        PlayerDefinition player;
        std::vector<EnemyDefinition> enemies;
        double spawnInterval = 1.2; // seconds between two enemies
        double playDepth = 12.0;    // distance from the camera to the plane the game is played on

        static GameAssets load(const std::string &path, RayTracer::Scene &scene);
    };
}

#endif
