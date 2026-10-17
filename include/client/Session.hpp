#ifndef SESSION_HPP_
#define SESSION_HPP_

#include "client/ArgumentParser.hpp"
#include "client/ClientSettings.hpp"
#include "client/GameClient.hpp"
#include "client/IClientMode.hpp"
#include "client/InputManager.hpp"
#include "client/ModelLibrary.hpp"
#include "client/SettingsPanel.hpp"
#include "core/Engine.hpp"
#include "core/Scene.hpp"
#include <map>
#include <memory>
#include <vector>

namespace Client {

    // The client program: owns the main loop, reads the input, runs the stack of modes and has
    // the engine render the scene they built.
    class Session {
        public:
            Session(const ClientConfig &config, RayTracer::Scene &scene, RayTracer::Engine &engine,
                const ModelLibrary &models, ServerLinkFactory connect);

            void run();

        private:
            void push(ModeId id);
            void pop();
            void clear();
            // false when it asks to quit
            bool apply(const Transition &transition);
            bool isInStack(const IClientMode *mode) const;

            ClientConfig _config;
            RayTracer::Scene &_scene;
            RayTracer::Engine &_engine;
            InputManager _input;
            ClientSettings _clientSettings;
            SettingsPanel _settings; // shared by the title screen and the menu

            std::map<ModeId, std::unique_ptr<IClientMode>> _modes;
            std::vector<IClientMode *> _stack; // the last one is on top
    };
}

#endif
