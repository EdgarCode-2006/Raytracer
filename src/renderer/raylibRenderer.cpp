#ifndef RAYLIB_RENDERER_HPP_
#define RAYLIB_RENDERER_HPP_

#include "renderer/Renderer.hpp"
#include "renderer/RaylibRenderer.hpp"
#include <cstring>

namespace Graphic {
    RaylibRenderer::~RaylibRenderer() {
        UnloadImage(_canvas);
        UnloadTexture(_texture);
        CloseWindow();
    }

    RaylibRenderer::RaylibRenderer(const int screenWidth, const int screenHeight) {
        SetTraceLogLevel(LOG_NONE);

        InitWindow(screenWidth, screenHeight, "RayTracer Window");
        SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);

        _canvas = GenImageColor(screenWidth, screenHeight, BLACK);
        this->_screenBuffer = (Color *)_canvas.data;

        _pixels.resize(screenWidth * screenHeight);

        _texture = LoadTextureFromImage(_canvas);
        SetTargetFPS(60);
    }


    void RaylibRenderer::pixelsToScreenBuffer(Pixel *pixels)
    {
        int h = GetScreenHeight();
        int w = GetScreenWidth();
        size_t totalBytes = h * w * sizeof(Color);

        std::memcpy(this->_screenBuffer, pixels, totalBytes);
    }

    void RaylibRenderer::renderPixelBuffer(Pixel* pixels)
    {
        this->pixelsToScreenBuffer(pixels);

        UpdateTexture(_texture, this->_screenBuffer);

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(_texture, 0, 0, WHITE);
            DrawFPS(10, 10);
        EndDrawing();
    }

    void RaylibRenderer::resizeWindow(int newWidth, int newHeight)
    {
        UnloadImage(this->_canvas);
        UnloadTexture(_texture);

        _canvas = GenImageColor(newWidth, newHeight, BLACK);
        _screenBuffer = (Color *)_canvas.data; 
        _pixels.resize(newWidth * newHeight);

        _texture = LoadTextureFromImage(_canvas);
    }
}

#endif