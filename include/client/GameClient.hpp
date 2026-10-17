#ifndef GAME_CLIENT_HPP_
#define GAME_CLIENT_HPP_

#include "client/IClientMode.hpp"
#include "client/IServerLink.hpp"
#include "client/SceneBuilder.hpp"
#include "common/InputState.hpp"
#include "common/WorldState.hpp"
#include "core/Scene.hpp"
#include <cstdint>
#include <functional>
#include <memory>

namespace Client {

    using ServerLinkFactory = std::function<std::unique_ptr<IServerLink>()>;

    // Connected to a server: sends it the player's input every frame and shows the world it sends
    // back. No game rule runs here. Under the menu it stays connected and keeps showing the
    // world, sending no button held (the server doesn't pause for one player).
    class GameClient : public AClientMode {
        public:
            GameClient(RayTracer::Scene &scene, const ModelLibrary &models, ServerLinkFactory connect);

            void onEnter() override;  // connects
            void onExit() override;   // disconnects
            void update(const InputManager &input, double dt) override;
            void updateCovered(double dt) override;
            void drawOverlay() override;

        private:
            // sends the input, then shows the newest world state
            void exchange(Common::InputState input, double dt);

            RayTracer::Scene &_scene;
            SceneBuilder _builder;
            ServerLinkFactory _connect;
            std::unique_ptr<IServerLink> _link;

            Common::WorldState _world; // the last state received, shown until a newer one comes
            uint32_t _inputSequence = 0;
    };
}

#endif
