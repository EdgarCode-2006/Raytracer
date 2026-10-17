#include "client/Session.hpp"
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
        : _config(config), _scene(scene), _engine(engine)
    {
        fitCameraToWorld(_scene.getCamera());

        _modes[ModeId::Title] = std::make_unique<TitleClient>(_scene);
        _modes[ModeId::Game] = std::make_unique<GameClient>(_scene, models, std::move(connect));
        changeMode(ModeId::Title);
    }

    void Session::changeMode(ModeId id)
    {
        if (_currentMode)
            _currentMode->onExit();
        _currentModeId = id;
        _currentMode = _modes.at(id).get();
        _currentMode->onEnter();
    }

    void Session::run()
    {
        auto lastTime = std::chrono::steady_clock::now();

        while (_engine.windowOpen()) {
            const auto now = std::chrono::steady_clock::now();
            const double dt = std::min(std::chrono::duration<double>(now - lastTime).count(), MAX_FRAME_TIME);
            lastTime = now;

            _input.update();
            _currentMode->update(_input, dt);
            _engine.renderFrame([this]() { _currentMode->drawOverlay(); });

            const ModeId next = _currentMode->next();
            if (next == ModeId::Quit)
                break;
            if (next != _currentModeId)
                changeMode(next);
        }
        _currentMode->onExit();
    }
}
