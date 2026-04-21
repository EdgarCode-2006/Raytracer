#ifndef RENDERER_HPP_
#define RENDERER_HPP_
#include <iostream>
#include <raylib.h>

class Renderer {
    public:
        void renderPixelBuffer(Color *pixel);

};


class RendererException : public std::exception {
    public:
        RendererException(const std::string& message) : message_(message) {}
        const char* what() const throw() { return message_.c_str(); }

    private:
        std::string message_;
};
#endif