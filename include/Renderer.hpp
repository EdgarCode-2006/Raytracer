#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <string>
#include <exception>
#include <raylib.h>
#include <vector>

#include "entities/Camera.hpp"

namespace Graphic {
    class Renderer {
        public:
            Renderer(const int screenHeight, const int screenWidth);
            ~Renderer();
            void renderPixelBuffer(Color *pixels);
            void checkWindowState(RayTracer::Camera &cam);
            void resizeCanvas(int newWidth, int newHeight);
            Color* getPixels() {return this->_pixels;};
        private:
            Color *_pixels;
            Image _canvas;
            Texture2D _texture;
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
