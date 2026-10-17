#ifndef SIMULATION_GAME_RULES_HPP_
#define SIMULATION_GAME_RULES_HPP_

#include "common/EntityType.hpp"
#include <string>
#include <vector>

namespace Simulation {

    struct PlayerRules {
        float speed = 9.0f; // world units per second
        float size = 1.0f;  // half of the ship's extent: keeps it inside the play area (later its hitbox)
    };

    struct EnemyRules {
        Common::EntityType type;
        float speed = 4.0f;     // to the left, world units per second
        float amplitude = 0.0f; // of the vertical wave, world units
        float frequency = 0.0f; // of the vertical wave, radians per second
        float size = 1.0f;      // half of its extent
    };

    // The gameplay values, read from assets/game/gameplay.cfg: the server's side of entities.cfg.
    struct GameRules {
        PlayerRules player;
        std::vector<EnemyRules> enemies;
        float spawnInterval = 1.2f; // seconds between two enemies

        static GameRules load(const std::string &path);
    };
}

#endif
