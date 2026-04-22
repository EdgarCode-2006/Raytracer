#ifndef IRENDERER_HPP_
#define IRENDERER_HPP_
#include "utils/Pixel.hpp"
#include <iostream>
#include <vector>

namespace Graphic {
    class IRenderer {
        public:
            virtual ~IRenderer() = default;
            virtual void resizeWindow(int newWidth, int newHeight) = 0;
            virtual void pixelsToScreenBuffer(Pixel *pixels) = 0;
            virtual void renderPixelBuffer(Pixel *pixels) = 0;
            virtual Pixel *getPixels() = 0;
            virtual bool windowOpen() = 0;
    };

    class ARenderer : public IRenderer {
        public:
            virtual ~ARenderer() = default;
            virtual void resizeWindow(int newWidth, int newHeight) = 0;
            virtual void pixelsToScreenBuffer(Pixel *pixels) = 0;
            virtual void renderPixelBuffer(Pixel *pixels) = 0;
            virtual Pixel *getPixels() override {return _pixels.data();};
            virtual bool windowOpen() = 0;
        protected:
            std::vector<Pixel> _pixels;
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