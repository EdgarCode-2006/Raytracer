#ifndef PLAYING_STATE_HPP_
#define PLAYING_STATE_HPP_

#include "game/IGameState.hpp"
#include "game/GameAssets.hpp"
#include "game/PlayArea.hpp"
#include "game/Player.hpp"
#include "game/Enemy.hpp"
#include <cstdint>
#include <random>
#include <vector>

namespace Game {

    // The player flies on a plane in front of the fixed camera while enemies come from the right.
    // Every frame the entities are written to the scene as instances, after the ones of the scene file.
    class PlayingState : public IGameState {
        public:
            PlayingState(RayTracer::Scene& scene, const GameAssets& assets);

            void onEnter() override;
            void onExit() override;
            void update(const Input::InputManager& input, double dt) override;
            void drawOverlay() override;
            StateId next() const noexcept override { return _next; }

        private:
            void spawnEnemy();
            void writeInstances();

            RayTracer::Scene& _scene;
            const GameAssets& _assets;
            StateId _next = StateId::Playing;

            PlayArea _area;
            uint32_t _firstInstance = 0; // the instances before it belong to the scene file
            Player _player;
            std::vector<Enemy> _enemies;
            double _spawnTimer = 0.0;
            std::mt19937 _rng{std::random_device{}()};
    };
}

#endif
