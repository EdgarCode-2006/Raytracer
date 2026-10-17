#include "client/SettingsPanel.hpp"
#include "imgui.h"
#include <array>
#include <raylib.h>
#include <string>

namespace Client {

    struct RebindableAction {
        Action action;
        const char *label;
    };

    // Esc (back) and Enter (confirm) drive the menus: they can't be rebound
    static constexpr std::array<RebindableAction, 5> REBINDABLE = {{
        {Action::MoveUp, "Move up"},
        {Action::MoveDown, "Move down"},
        {Action::MoveLeft, "Move left"},
        {Action::MoveRight, "Move right"},
        {Action::Fire, "Fire"},
    }};

    void SettingsPanel::draw()
    {
        if (_capture)
            captureKey();

        if (ImGui::BeginTabBar("Settings")) {
            if (ImGui::BeginTabItem("Controls")) {
                drawControls();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Graphics")) {
                drawGraphics();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Accessibility")) {
                drawAccessibility();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    void SettingsPanel::captureKey()
    {
        const int key = _input.getKeyPressed();
        // F12 is raylib's screenshot key: it keeps waiting
        if (key == 0 || key == KEY_F12)
            return;
        if (key == KEY_BACKSPACE || key == KEY_DELETE)
            _input.clearKey(_capture->action, _capture->slot);
        else if (key != KEY_ESCAPE && key != KEY_ENTER)
            _input.setKey(_capture->action, _capture->slot, key);
        _refocus = _capture;
        _capture.reset();
        // the key was taken before ImGui saw it, its navigation can come back from the next frame
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    }

    void SettingsPanel::drawControls()
    {
        const ImVec2 slotSize(120, 0);

        if (ImGui::BeginTable("Keys", 1 + InputManager::MAX_KEYS_PER_ACTION, ImGuiTableFlags_SizingFixedFit)) {
            for (const auto &[action, label] : REBINDABLE) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(label);

                const auto &keys = _input.getKeys(action);
                for (std::size_t slot = 0; slot < InputManager::MAX_KEYS_PER_ACTION; slot++) {
                    ImGui::TableNextColumn();
                    const bool capturing = _capture && _capture->action == action && _capture->slot == slot;
                    std::string text = capturing ? "press a key..." : slot < keys.size() ? InputManager::keyName(keys[slot]) : "-";
                    text += "##" + std::string(label) + std::to_string(slot);
                    if (_refocus && _refocus->action == action && _refocus->slot == slot) {
                        ImGui::SetKeyboardFocusHere();
                        _refocus.reset();
                    }
                    if (ImGui::Button(text.c_str(), slotSize) && !_capture) {
                        _capture = Capture{action, slot};
                        // until the key comes, Enter and the arrows must not drive the menu
                        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
                    }
                }
            }
            ImGui::EndTable();
        }

        // the same text while waiting for a key, so that the window keeps its size
        ImGui::Spacing();
        ImGui::TextDisabled("Click a key, then press the new one.");
        ImGui::TextDisabled("Backspace removes it, Esc cancels. Esc and Enter drive the menus.");
        if (ImGui::Button("Reset to default") && !_capture)
            _input.resetKeys();
    }

    void SettingsPanel::drawGraphics()
    {
        Graphic::IRenderer &renderer = _engine.getRenderer();
        bool showFps = renderer.isShowingFps();
        if (ImGui::Checkbox("Show FPS", &showFps))
            renderer.setShowFps(showFps);

        ImGui::Spacing();
        ImGui::SeparatorText("Shader features");
        bool anyDebug = false;
        for (auto &feature : _engine.getFeatures()) {
            if (feature->getType() == DEBUG) {
                anyDebug = true;
                continue;
            }
            bool active = feature->isActive();
            if (ImGui::Checkbox(feature->getName().c_str(), &active))
                feature->setActivity(active);
        }
        ImGui::TextDisabled("Path tracing and ambient occlusion are noisy at one sample per pixel");

        if (anyDebug && ImGui::CollapsingHeader("Debug views")) {
            for (auto &feature : _engine.getFeatures()) {
                if (feature->getType() != DEBUG)
                    continue;
                bool active = feature->isActive();
                if (ImGui::Checkbox(feature->getName().c_str(), &active))
                    feature->setActivity(active);
            }
        }
    }

    void SettingsPanel::drawAccessibility()
    {
        ImGui::Checkbox("High contrast", &_settings.highContrast);
        ImGui::TextDisabled("Ships as flat bright shapes, a color per type, over a dimmed background.");
        ImGui::TextDisabled("Player in cyan, enemies in warm colors (models.cfg, contrastColor).");

        // the flat shapes are drawn by the Emission feature
        if (_settings.highContrast) {
            for (const auto &feature : _engine.getFeatures()) {
                if (feature->getName() == "Emission" && !feature->isActive()) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Turn Emission on (Graphics) to see the ships.");
                }
            }
        }
    }
}
