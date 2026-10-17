#ifndef COMMON_WORLD_STATE_HPP_
#define COMMON_WORLD_STATE_HPP_

#include "common/EntityType.hpp"
#include "common/Vec2.hpp"
#include <cstdint>
#include <vector>

namespace Common {

    struct EntityState {
        uint32_t id;      // unique for the whole game: the client follows an entity with it
        EntityType type;
        Vec2 position;
        Vec2 velocity;    // world units per second, for the client to animate (and later interpolate)
    };

    // What the server sends to a client every tick. Only gameplay: how an entity looks (model,
    // rotation, glow) is decided by the client.
    struct WorldState {
        uint32_t tick = 0;
        uint32_t playerId = 0;          // the entity of the client receiving this state
        uint32_t lastInputSequence = 0; // the last input of that client the server applied
        std::vector<EntityState> entities;
    };
}

#endif
