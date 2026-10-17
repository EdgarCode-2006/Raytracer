#include "renderer/RaylibRenderer.hpp"
#include "rlImGui.h"
#include "imgui.h"
#include <cstring>

namespace Graphic {
    RaylibRenderer::~RaylibRenderer() {
        rlImGuiShutdown();
        UnloadImage(_canvas);
        UnloadTexture(_texture);
        CloseWindow();
    }

    RaylibRenderer::RaylibRenderer(const int screenWidth, const int screenHeight)
        : ARenderer(screenWidth, screenHeight)
    {
        SetTraceLogLevel(LOG_NONE);

        InitWindow(screenWidth, screenHeight, "R-Type");
        SetWindowState(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
        // Escape is a game action (back to the menu), only closing the window quits
        SetExitKey(KEY_NULL);

        _canvas = GenImageColor(screenWidth, screenHeight, BLACK);
        this->_screenBuffer = (Color *)_canvas.data;

        _texture = LoadTextureFromImage(_canvas);
        SetTargetFPS(60);
        rlImGuiSetup(true);

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        // the menus are driven with the keyboard: without it, the first key only shows the
        // highlight of the focused button and pressing it takes a second Enter
        io.ConfigNavCursorVisibleAlways = true;
        io.IniFilename = nullptr; // the windows are placed by code, nothing to remember
    }

    void RaylibRenderer::pixelsToScreenBuffer(Pixel *pixels)
    {
        std::memcpy(this->_screenBuffer, pixels, (size_t)_width * _height * sizeof(Color));
    }

    void RaylibRenderer::renderPixelBuffer(Pixel* pixels, const std::function<void()> &drawOverlay)
    {
        this->pixelsToScreenBuffer(pixels);

        UpdateTexture(_texture, this->_screenBuffer);

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(_texture, 0, 0, WHITE);
            DrawFPS(10, 10);
            if (drawOverlay) {
                rlImGuiBegin();
                drawOverlay();
                rlImGuiEnd();
            }
        EndDrawing();
    }
}
