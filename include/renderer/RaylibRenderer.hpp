#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <string>
#include <exception>
#include <raylib.h>
#include <vector>
#include "utils/Pixel.hpp"

#include "entities/Camera.hpp"
#include "renderer/Renderer.hpp"
#include "GUI.hpp"

namespace Graphic {
    class RaylibRenderer : public ARenderer {
        public:
            RaylibRenderer(const int screenWidth, const int screenHeight);
            ~RaylibRenderer();
            void resizeWindow(int newWidth, int newHeight) override;
            void pixelsToScreenBuffer(Pixel *pixels) override;
            void renderPixelBuffer(Pixel *pixels, RayTracer::Scene& scene, RayTracer::Camera& cam) override;
            bool windowOpen() override {return !WindowShouldClose();};
            void applyBlur(Pixel *pixels, int h, int w);
        private:
            Color *_screenBuffer;
            Image _canvas;
            Texture2D _texture;
            GUI _gui;
    };
}

#endif
