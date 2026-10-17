#ifndef SESSION_HPP_
#define SESSION_HPP_

#include "client/ArgumentParser.hpp"
#include "client/GameClient.hpp"
#include "client/IClientMode.hpp"
#include "client/InputManager.hpp"
#include "client/ModelLibrary.hpp"
#include "core/Engine.hpp"
#include "core/Scene.hpp"
#include <map>
#include <memory>

namespace Client {

    // The client program: owns the main loop, reads the input, runs the current mode and has the
    // engine render the scene the mode built.
    class Session {
        public:
            Session(const ClientConfig &config, RayTracer::Scene &scene, RayTracer::Engine &engine,
                const ModelLibrary &models, ServerLinkFactory connect);

            void run();

        private:
            void changeMode(ModeId id);

            ClientConfig _config;
            RayTracer::Scene &_scene;
            RayTracer::Engine &_engine;
            InputManager _input;

            std::map<ModeId, std::unique_ptr<IClientMode>> _modes;
            ModeId _currentModeId = ModeId::Title;
            IClientMode *_currentMode = nullptr;
    };
}

#endif
