#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "game/GameAssets.hpp"
#include "game/PlayArea.hpp"
#include "input/InputManager.hpp"

namespace Game {

    class Player {
        public:
            explicit Player(const PlayerDefinition &definition) : _definition(&definition) {}

            void reset(const PlayArea &area);
            void update(const Input::InputManager &input, double dt, const PlayArea &area);
            void addToScene(RayTracer::Scene &scene) const;

            const Math::Vector3D &getPosition() const noexcept { return _transform.position; }

        private:
            const PlayerDefinition *_definition;
            Transform _transform;
    };
}

#endif
