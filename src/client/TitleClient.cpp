#include "client/TitleClient.hpp"
#include "imgui.h"

namespace Client {

    void TitleClient::update(const InputManager &input, double dt)
    {
        _scene.scrollBackground(dt);
        // Enter and the arrow keys already drive the buttons through ImGui keyboard navigation
        if (!input.isPressed(Action::Back) || _settings.isCapturingKey())
            return;
        if (_inSettings)
            _inSettings = false;
        else
            request(Transition::quit());
    }

    void TitleClient::drawOverlay()
    {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        const ImVec2 buttonSize(200, 0);

        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin(_inSettings ? "Settings##Title" : "R-Type", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_AlwaysAutoResize);

        const char *title = _inSettings ? "SETTINGS" : "R-TYPE";
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(title).x) * 0.5f);
        ImGui::TextUnformatted(title);
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
            if (ImGui::Button("Play", buttonSize))
                request(Transition::switchTo(ModeId::Game));
            if (ImGui::Button("Settings", buttonSize))
                _inSettings = true;
            if (ImGui::Button("Quit", buttonSize))
                request(Transition::quit());
        }

        ImGui::End();
    }
}
