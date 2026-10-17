#include "client/Session.hpp"
#include "client/MenuClient.hpp"
#include "client/TitleClient.hpp"
#include "common/World.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <numbers>

namespace Client {

    // a long stall (window dragged, breakpoint) must not make everything jump
    static constexpr double MAX_FRAME_TIME = 0.1;

    // Puts the camera in front of the plane z = 0 so that it sees exactly the play area.
    static void fitCameraToWorld(RayTracer::Camera &cam)
    {
        const double halfFov = cam.getFov() / 2.0 * std::numbers::pi / 180.0;
        const double distance = Common::WORLD_HALF_HEIGHT / std::tan(halfFov);
        cam.setPosition(Math::Vector3D(0.0, 0.0, -distance));
        cam.setOrientation(0.0, 0.0);
    }

    Session::Session(const ClientConfig &config, RayTracer::Scene &scene, RayTracer::Engine &engine,
        const ModelLibrary &models, ServerLinkFactory connect)
        : _config(config), _scene(scene), _engine(engine), _settings(_input, engine, _clientSettings)
    {
        fitCameraToWorld(_scene.getCamera());

        _modes[ModeId::Title] = std::make_unique<TitleClient>(_scene, _settings);
        _modes[ModeId::Game] = std::make_unique<GameClient>(_scene, models, _clientSettings, std::move(connect));
        _modes[ModeId::Menu] = std::make_unique<MenuClient>(_settings);
        push(ModeId::Title);
    }

    bool Session::isInStack(const IClientMode *mode) const
    {
        return std::find(_stack.begin(), _stack.end(), mode) != _stack.end();
    }

    void Session::push(ModeId id)
    {
        IClientMode *mode = _modes.at(id).get();
        if (isInStack(mode))
            return; // a mode is in the stack once at most
        _stack.push_back(mode);
        mode->onEnter();
    }

    void Session::pop()
    {
        if (_stack.size() <= 1)
            return; // there's always a mode running
        _stack.back()->onExit();
        _stack.pop_back();
    }

    void Session::clear()
    {
        // from the top: the menu leaves before the game it covers
        while (!_stack.empty()) {
            _stack.back()->onExit();
            _stack.pop_back();
        }
    }

    bool Session::apply(const Transition &transition)
    {
        switch (transition.kind) {
            case Transition::Kind::None: break;
            case Transition::Kind::Push: push(transition.target); break;
            case Transition::Kind::Pop: pop(); break;
            case Transition::Kind::Switch: clear(); push(transition.target); break;
            case Transition::Kind::Quit: return false;
        }
        return true;
    }

    void Session::run()
    {
        auto lastTime = std::chrono::steady_clock::now();
        bool running = true;

        while (running && _engine.windowOpen()) {
            const auto now = std::chrono::steady_clock::now();
            const double dt = std::min(std::chrono::duration<double>(now - lastTime).count(), MAX_FRAME_TIME);
            lastTime = now;

            _input.update();
            _scene.setBackgroundBrightness(_clientSettings.highContrast ? HIGH_CONTRAST_BACKGROUND : 1.0);
            for (size_t i = 0; i + 1 < _stack.size(); i++)
                _stack[i]->updateCovered(dt);
            _stack.back()->update(_input, dt);

            _engine.renderFrame([this]() {
                for (IClientMode *mode : _stack)
                    mode->drawOverlay();
            });

            // from the top; a mode a previous transition removed has nothing left to ask
            const std::vector<IClientMode *> modes(_stack.rbegin(), _stack.rend());
            for (IClientMode *mode : modes) {
                const Transition transition = mode->takeTransition();
                if (isInStack(mode) && !apply(transition)) {
                    running = false;
                    break;
                }
            }
        }
        clear();
    }
}
