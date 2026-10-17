#include "client/MenuClient.hpp"
#include "imgui.h"

namespace Client {

    void MenuClient::update(const InputManager &input, double dt)
    {
        (void)dt;
        if (!input.isPressed(Action::Back) || _settings.isCapturingKey())
            return;
        if (_inSettings)
            _inSettings = false;
        else
            request(Transition::pop());
    }

    void MenuClient::drawOverlay()
    {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        const ImVec2 buttonSize(220, 0);

        // dims the game: drawn over the image, under the ImGui windows
        ImGui::GetBackgroundDrawList()->AddRectFilled(viewport->Pos,
            ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y), IM_COL32(0, 0, 0, 140));

        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin(_inSettings ? "Settings##Menu" : "Menu", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_AlwaysAutoResize);

        const char *title = _inSettings ? "SETTINGS" : "MENU";
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(title).x) * 0.5f);
        ImGui::TextUnformatted(title);
        ImGui::TextDisabled("The game goes on while the menu is open");
        ImGui::Separator();
        ImGui::Spacing();

        if (_inSettings) {
            _settings.draw();
            ImGui::Spacing();
            if (ImGui::Button("Back", buttonSize) && !_settings.isCapturingKey())
                _inSettings = false;
        } else {
            if (ImGui::IsWindowAppearing())
                ImGui::SetKeyboardFocusHere();
            if (ImGui::Button("Resume", buttonSize))
                request(Transition::pop());
            if (ImGui::Button("Settings", buttonSize))
                _inSettings = true;
            if (ImGui::Button("Leave the game", buttonSize))
                request(Transition::switchTo(ModeId::Title));
            if (ImGui::Button("Quit", buttonSize))
                request(Transition::quit());
        }

        ImGui::End();
    }
}
