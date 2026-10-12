#include "game/PlayingState.hpp"
#include "imgui.h"

namespace Game {

    void PlayingState::update(const Input::InputManager& input)
    {
        if (input.isPressed(Input::Action::Back))
            _next = StateId::Menu;
    }

    void PlayingState::drawOverlay()
    {
        ImGui::SetNextWindowPos(ImVec2(10, 40), ImGuiCond_Always);
        ImGui::Begin("Playing", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoInputs);
        ImGui::TextUnformatted("Game not implemented yet - Esc to go back to the menu");
        ImGui::End();
    }
}
