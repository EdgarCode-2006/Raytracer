#ifndef IRENDERER_HPP_
#define IRENDERER_HPP_
#include "utils/Pixel.hpp"
#include <functional>
#include <exception>
#include <string>
#include <vector>

namespace Graphic {
    class IRenderer {
        public:
            virtual ~IRenderer() = default;
            virtual int getWidth() const noexcept = 0;
            virtual int getHeight() const noexcept = 0;
            virtual void pixelsToScreenBuffer(Pixel *pixels) = 0;
            // drawOverlay makes the ImGui calls drawn over the frame
            virtual void renderPixelBuffer(Pixel *pixels, const std::function<void()> &drawOverlay) = 0;
            virtual bool windowOpen() = 0;
    };

    class ARenderer : public IRenderer {
        public:
            ARenderer(int width, int height) : _width(width), _height(height) {}
            virtual ~ARenderer() = default;
            int getWidth() const noexcept override { return _width; }
            int getHeight() const noexcept override { return _height; }
        protected:
            int _width;
            int _height;
    };

    class RendererException : public std::exception {
        public:
            RendererException(const std::string& message) : _message(message) {}
            const char* what() const noexcept override { return _message.c_str(); }
        private:
            std::string _message;
    };
}
#endif
