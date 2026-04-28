#ifndef PIXEL_HPP_
#define PIXEL_HPP_

namespace Graphic {
    struct Pixel {
        unsigned char _r;
        unsigned char _g;
        unsigned char _b;
        unsigned char _a;

        Pixel() : _r(0), _g(0), _b(0), _a(255) {}
        Pixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a) :
            _r(r), _g(g), _b(b), _a(a) {}
        Pixel(unsigned char r, unsigned char g, unsigned char b) :
            _r(r), _g(g), _b(b), _a(255) {}
    };
}
#endif