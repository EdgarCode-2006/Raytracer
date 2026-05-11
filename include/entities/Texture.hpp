#ifndef TEXTURE_HPP_
#define TEXTURE_HPP_
#include <string>
#include "utils/Pixel.hpp"
#include <iostream>

namespace RayTracer {
    class Texture {
        public:
            Texture(const std::string &fileName);
            Texture() : _textureData(nullptr), _width(0), _height(0) {};
            ~Texture();

            Texture(const Texture&) = delete;
            Texture& operator=(const Texture&) = delete;

            void loadTextureFromFile(const std::string &fileName);
            void clearTextureData();
            Graphic::Pixel sampleUVColor(double u, double v);
        private:
            unsigned char* _textureData;
            int _width;
            int _height;
    };
}
#endif