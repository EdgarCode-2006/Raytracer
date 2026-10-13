#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include "game/GameAssets.hpp"
#include "game/PlayArea.hpp"

namespace Game {

    // Flies from the right of the screen to the left, waving up and down.
    class Enemy {
        public:
            Enemy(const EnemyDefinition &definition, const Math::Vector3D &spawn);

            void update(double dt);
            bool isPast(const PlayArea &area) const noexcept;
            void addToScene(RayTracer::Scene &scene) const;

        private:
            const EnemyDefinition *_definition;
            Transform _transform;
            double _baseY;
            double _age = 0.0;
    };
}

#endif
