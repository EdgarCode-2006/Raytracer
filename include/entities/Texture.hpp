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
            // u and v repeat every 1: the texture tiles in both directions
            Graphic::Pixel sampleWrapped(double u, double v) const;
            bool isLoaded() const noexcept { return _textureData != nullptr; }
            int getWidth() const noexcept { return _width; }
            int getHeight() const noexcept { return _height; }
        private:
            unsigned char* _textureData;
            int _width;
            int _height;
    };
}
#endif