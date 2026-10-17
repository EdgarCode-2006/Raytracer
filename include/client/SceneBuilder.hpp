#ifndef SCENE_BUILDER_HPP_
#define SCENE_BUILDER_HPP_

#include "client/ClientSettings.hpp"
#include "client/ModelLibrary.hpp"
#include "common/WorldState.hpp"
#include "core/Scene.hpp"
#include <cstdint>
#include <unordered_map>

namespace Client {

    // Turns what the server sends into what the raytracer renders: one model per entity, on the
    // plane z = 0 (the camera is placed to see the whole play area there). The animation that
    // isn't gameplay (rolling, spinning) is added here, kept per entity id between states.
    class SceneBuilder {
        public:
            SceneBuilder(RayTracer::Scene &scene, const ModelLibrary &models, const ClientSettings &settings);

            // rewrites the entities' instances; dt animates them
            void apply(const Common::WorldState &world, double dt);
            // removes them, leaving the scene as its file describes it
            void clear();

        private:
            struct Animation {
                double bank = 0.0;
                double spin = 0.0;
                uint32_t lastSeen = 0;
            };

            RayTracer::Scene &_scene;
            const ModelLibrary &_models;
            const ClientSettings &_settings;
            uint32_t _firstInstance; // the instances before it belong to the scene file
            std::unordered_map<uint32_t, Animation> _animations;
            uint32_t _applyCount = 0;
    };
}

#endif
