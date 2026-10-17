#include "client/GameClient.hpp"
#include "imgui.h"

namespace Client {

    GameClient::GameClient(RayTracer::Scene &scene, const ModelLibrary &models, const ClientSettings &settings,
        ServerLinkFactory connect)
        : _scene(scene), _builder(scene, models, settings), _connect(std::move(connect))
    {
    }

    void GameClient::onEnter()
    {
        _world = {};
        _inputSequence = 0;
        _link = _connect();
    }

    void GameClient::onExit()
    {
        _link.reset();
        _builder.clear();
    }

    void GameClient::update(const InputManager &input, double dt)
    {
        if (input.isPressed(Action::Back)) {
            request(Transition::push(ModeId::Menu));
            exchange(Common::InputState{}, dt); // the menu opens: let go of everything now
            return;
        }
        exchange(input.toInputState(0), dt);
    }

    void GameClient::updateCovered(double dt)
    {
        exchange(Common::InputState{}, dt);
    }

    void GameClient::exchange(Common::InputState input, double dt)
    {
        if (!_link->isConnected()) {
            request(Transition::switchTo(ModeId::Title));
            return;
        }
        _scene.scrollBackground(dt);
        input.sequence = ++_inputSequence;
        _link->sendInput(input);
        if (auto world = _link->takeLatestState())
            _world = std::move(*world);
        _builder.apply(_world, dt);
    }

    void GameClient::drawOverlay()
    {
        ImGui::SetNextWindowPos(ImVec2(10, 40), ImGuiCond_Always);
        ImGui::Begin("Playing", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::Text("%s - tick %u", _link->describe().c_str(), _world.tick);
        ImGui::Text("Entities: %zu", _world.entities.size());
        ImGui::TextUnformatted("Arrows / WASD to move - Esc for the menu");
        ImGui::End();
    }
}
