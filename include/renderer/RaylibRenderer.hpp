#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <raylib.h>
#include "utils/Pixel.hpp"
#include "renderer/Renderer.hpp"

namespace Graphic {
    // A window of fixed size: the play area of the game never changes.
    class RaylibRenderer : public ARenderer {
        public:
            RaylibRenderer(const int screenWidth, const int screenHeight);
            ~RaylibRenderer();
            void pixelsToScreenBuffer(Pixel *pixels) override;
            void renderPixelBuffer(Pixel *pixels, const std::function<void()> &drawOverlay) override;
            bool windowOpen() override {return !WindowShouldClose();};
        private:
            Color *_screenBuffer;
            Image _canvas;
            Texture2D _texture;
    };
}

#endif
