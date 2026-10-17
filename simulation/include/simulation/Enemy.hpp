#ifndef SIMULATION_ENEMY_HPP_
#define SIMULATION_ENEMY_HPP_

#include "common/WorldState.hpp"
#include "simulation/GameRules.hpp"

namespace Simulation {

    // Flies from the right of the play area to the left, waving up and down.
    class Enemy {
        public:
            Enemy(uint32_t id, const EnemyRules &rules, Common::Vec2 spawn);

            void update(float dt);
            bool isPast() const noexcept; // out of the play area on the left

            Common::EntityState state() const noexcept;

        private:
            uint32_t _id;
            const EnemyRules *_rules;
            Common::Vec2 _position;
            Common::Vec2 _velocity;
            float _baseY;
            float _age = 0.0f;
    };
}

#endif
