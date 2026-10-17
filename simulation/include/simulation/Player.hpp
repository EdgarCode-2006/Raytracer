#ifndef SIMULATION_PLAYER_HPP_
#define SIMULATION_PLAYER_HPP_

#include "common/InputState.hpp"
#include "common/WorldState.hpp"
#include "simulation/GameRules.hpp"

namespace Simulation {

    class Player {
        public:
            Player(uint32_t id, const PlayerRules &rules, Common::Vec2 start);

            void setInput(const Common::InputState &input) noexcept { _input = input; }
            const Common::InputState &getInput() const noexcept { return _input; }
            void update(float dt);

            uint32_t getId() const noexcept { return _id; }
            Common::EntityState state() const noexcept;

        private:
            uint32_t _id;
            const PlayerRules *_rules;
            Common::Vec2 _position;
            Common::Vec2 _velocity;
            Common::InputState _input;
    };
}

#endif
