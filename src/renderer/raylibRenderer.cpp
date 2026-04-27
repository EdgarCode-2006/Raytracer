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

    void RaylibRenderer::applyBlur(Pixel *pixels, int h, int w)
    {
        std::vector<Pixel> source = _pixels;
        float strength = 0.4;

        for (int y = 1; y < h - 1; y++) {
            for (int x = 1; x < w - 1; x++) {
                int r, g, b = 0;
                // [1, 2, 1]
                // [2, 4, 2] / 16
                // [1, 2, 1]
                auto getP = [&](int dx, int dy) { return source[(y + dy) * w + (x + dx)]; };

                unsigned char blurR = (getP(-1,-1)._r * 1 + getP(0,-1)._r * 2 + getP(1,-1)._r * 1 +
                    getP(-1, 0)._r * 2 + getP(0, 0)._r * 4 + getP(1, 0)._r * 2 +
                    getP(-1, 1)._r * 1 + getP(0, 1)._r * 2 + getP(1, 1)._r * 1) >> 4;

                unsigned char blurG = (getP(-1,-1)._g * 1 + getP(0,-1)._g * 2 + getP(1,-1)._g * 1 +
                    getP(-1, 0)._g * 2 + getP(0, 0)._g * 4 + getP(1, 0)._g * 2 +
                    getP(-1, 1)._g * 1 + getP(0, 1)._g * 2 + getP(1, 1)._g * 1) >> 4;

                unsigned char blurB = (getP(-1,-1)._b * 1 + getP(0,-1)._b * 2 + getP(1,-1)._b * 1 +
                    getP(-1, 0)._b * 2 + getP(0, 0)._b * 4 + getP(1, 0)._b * 2 +
                    getP(-1, 1)._b * 1 + getP(0, 1)._b * 2 + getP(1, 1)._b * 1) >> 4;

                Pixel original = getP(0, 0);

                unsigned char finalR = original._r + (unsigned char)((blurR - original._r) * strength);
                unsigned char finalG = original._g + (unsigned char)((blurG - original._g) * strength);
                unsigned char finalB = original._b + (unsigned char)((blurB - original._b) * strength);

                pixels[y * w + x] = Pixel(finalR, finalG, finalB);
            }
        }
    }

    void RaylibRenderer::pixelsToScreenBuffer(Pixel *pixels)
    {
        int h = GetScreenHeight();
        int w = GetScreenWidth();
        size_t totalBytes = h * w * sizeof(Color);

        // this->applyBlur(pixels, h, w);
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