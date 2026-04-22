#include "Renderer.hpp"

namespace Graphic {
    Renderer::~Renderer() {
        UnloadImage(_canvas);
        UnloadTexture(_texture);
        CloseWindow();
    }

    Renderer::Renderer(const int screenWidth, const int screenHeight) {
        SetTraceLogLevel(LOG_NONE);

        InitWindow(screenWidth * 4, screenHeight * 4, "RayTracer Window");
        SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);

        _canvas = GenImageColor(screenWidth, screenHeight, BLACK);

        this->_pixels = (Color *)_canvas.data;

        _texture = LoadTextureFromImage(_canvas);
        SetTargetFPS(60);
    }

    void Renderer::renderPixelBuffer(Color *pixels) {
        int h = GetScreenHeight();
        int w = GetScreenWidth();

        UpdateTexture(_texture, pixels);

        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            // Draw the texture covering the whole screen
            DrawTextureEx(_texture, (Vector2){0, 0}, 0.0f, 4.0f, WHITE);
            
            DrawFPS(10, 10);

        EndDrawing();
    }

    void Renderer::resizeCanvas(int newWidth, int newHeight) {
        UnloadImage(_canvas);
        UnloadTexture(_texture);

        _canvas = GenImageColor(newWidth, newHeight, BLACK);
        _pixels = (Color *)_canvas.data;

        _texture = LoadTextureFromImage(_canvas);
    }
}