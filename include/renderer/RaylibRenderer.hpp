#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <string>
#include <exception>
#include <raylib.h>
#include <vector>
#include "utils/Pixel.hpp"

#include "entities/Camera.hpp"
#include "renderer/Renderer.hpp"

namespace Graphic {
    class RaylibRenderer : public ARenderer {
        public:
            RaylibRenderer(const int screenWidth, const int screenHeight);
            ~RaylibRenderer();
            void resizeWindow(int newWicdth, int newHeight) override;
            void pixelsToScreenBuffer(Pixel *pixels) override;
            void renderPixelBuffer(Pixel *pixels) override;
            bool windowOpen() override {return !WindowShouldClose();};
        private:
            Color *_screenBuffer;
            Image _canvas;
            Texture2D _texture;
    };
}

#endif
