#ifndef SIMULATION_GAME_SIMULATION_HPP_
#define SIMULATION_GAME_SIMULATION_HPP_

#include "common/InputState.hpp"
#include "common/WorldState.hpp"
#include "simulation/Enemy.hpp"
#include "simulation/GameRules.hpp"
#include "simulation/Player.hpp"
#include <cstdint>
#include <random>
#include <vector>

namespace Simulation {

    // The whole game, without any rendering, window or network: what the server runs.
    // It is not thread safe, whoever runs it calls it from one thread.
    class GameSimulation {
        public:
            explicit GameSimulation(GameRules rules, uint32_t seed = std::random_device{}());

            uint32_t addPlayer();             // the id of the new player's entity
            void removePlayer(uint32_t playerId);
            void setInput(uint32_t playerId, const Common::InputState &input);

            void step(float dt);

            // the world as seen by one player (playerId and lastInputSequence are theirs)
            Common::WorldState state(uint32_t playerId) const;
            uint32_t getTick() const noexcept { return _tick; }

        private:
            void spawnEnemy();

            GameRules _rules;
            std::vector<Player> _players;
            std::vector<Enemy> _enemies;
            uint32_t _nextId = 1;
            uint32_t _tick = 0;
            float _spawnTimer = 0.0f;
            std::mt19937 _rng;
    };
}

#endif
