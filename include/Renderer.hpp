#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <string>
#include <exception>
#include <raylib.h>
#include <vector>

namespace Graphic {
    class Renderer {
        public:
            void renderPixelBuffer(Color *pixels, int width, int height);
            int getScreenHeight();
            int getScreenWidth();
            Color* getPixels();
        private:
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
