#include "simulation/GameSimulation.hpp"
#include "common/World.hpp"
#include <algorithm>

namespace Simulation {

    // how far apart the players start, one above the other
    static constexpr float PLAYER_SPACING = 2.0f;

    GameSimulation::GameSimulation(GameRules rules, uint32_t seed)
        : _rules(std::move(rules)), _rng(seed)
    {
    }

    uint32_t GameSimulation::addPlayer()
    {
        // 0, +2, -2, +4, -4...
        const int index = static_cast<int>(_players.size());
        const float offset = ((index + 1) / 2) * PLAYER_SPACING * (index % 2 ? 1.0f : -1.0f);
        const float maxY = Common::WORLD_HALF_HEIGHT - _rules.player.size;
        const Common::Vec2 start{-Common::WORLD_HALF_WIDTH * 0.6f, std::clamp(offset, -maxY, maxY)};

        const uint32_t id = _nextId++;
        _players.emplace_back(id, _rules.player, start);
        return id;
    }

    void GameSimulation::removePlayer(uint32_t playerId)
    {
        std::erase_if(_players, [playerId](const Player &p) { return p.getId() == playerId; });
    }

    void GameSimulation::setInput(uint32_t playerId, const Common::InputState &input)
    {
        for (auto &player : _players) {
            // an older input arriving late must not replace a newer one
            if (player.getId() == playerId && input.sequence >= player.getInput().sequence)
                player.setInput(input);
        }
    }

    void GameSimulation::step(float dt)
    {
        _tick++;
        for (auto &player : _players)
            player.update(dt);

        _spawnTimer -= dt;
        if (_spawnTimer <= 0.0f) {
            spawnEnemy();
            _spawnTimer += _rules.spawnInterval;
        }
        for (auto &enemy : _enemies)
            enemy.update(dt);
        std::erase_if(_enemies, [](const Enemy &enemy) { return enemy.isPast(); });
    }

    void GameSimulation::spawnEnemy()
    {
        std::uniform_int_distribution<size_t> pickType(0, _rules.enemies.size() - 1);
        const EnemyRules &type = _rules.enemies[pickType(_rng)];

        // keep the whole wave inside the play area
        const float limit = std::max(0.0f, Common::WORLD_HALF_HEIGHT - type.amplitude - type.size);
        std::uniform_real_distribution<float> pickY(-limit, limit);

        _enemies.emplace_back(_nextId++, type, Common::Vec2{Common::WORLD_HALF_WIDTH + 2.0f * type.size, pickY(_rng)});
    }

    Common::WorldState GameSimulation::state(uint32_t playerId) const
    {
        Common::WorldState world;
        world.tick = _tick;
        world.playerId = playerId;
        world.entities.reserve(_players.size() + _enemies.size());
        for (const auto &player : _players) {
            world.entities.push_back(player.state());
            if (player.getId() == playerId)
                world.lastInputSequence = player.getInput().sequence;
        }
        for (const auto &enemy : _enemies)
            world.entities.push_back(enemy.state());
        return world;
    }
}
