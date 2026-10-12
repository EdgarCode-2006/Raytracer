#include "game/MenuState.hpp"
#include "imgui.h"

namespace Game {

    void MenuState::update(const Input::InputManager& input)
    {
        // Enter and the arrow keys already drive the buttons through ImGui keyboard navigation
        if (input.isPressed(Input::Action::Back))
            _next = StateId::Quit;
    }

    void MenuState::drawOverlay()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 buttonSize(200, 0);

        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("R-Type", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_AlwaysAutoResize);

        const char* title = "R-TYPE";
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(title).x) * 0.5f);
        ImGui::TextUnformatted(title);
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere();
        if (ImGui::Button("Play", buttonSize))
            _next = StateId::Playing;
        if (ImGui::Button("Quit", buttonSize))
            _next = StateId::Quit;

        ImGui::End();
    }
}
