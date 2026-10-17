#ifndef COMMON_WORLD_HPP_
#define COMMON_WORLD_HPP_

namespace Common {

    // The play area, in world units, centered on (0, 0). The server keeps the entities in it and
    // the client places its camera to show exactly this rectangle (16:9).
    inline constexpr float WORLD_HALF_WIDTH = 12.0f;
    inline constexpr float WORLD_HALF_HEIGHT = 6.75f;

    // How many times per second the server updates the world and sends it.
    inline constexpr int TICK_RATE = 60;
}

#endif
