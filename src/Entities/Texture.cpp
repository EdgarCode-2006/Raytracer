#include "entities/Texture.hpp"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace RayTracer
{
    Texture::Texture(const std::string &fileName) {
        this->loadTextureFromFile(fileName);
    }
    Texture::~Texture() {
        this->clearTextureData();
    }

    void Texture::clearTextureData()
    {
        if (!this->_textureData)
            return;
        stbi_image_free(_textureData);
        this->_textureData = nullptr;
    }
    void Texture::loadTextureFromFile(const std::string &fileName)
    {
        if (this->_textureData != nullptr)
            this->clearTextureData();
        int channels;
        _textureData = stbi_load(fileName.c_str(), &_width, &_height, &channels, 3);
        if (!_textureData) {
            std::cerr << "ERROR: Could not load texture file: " << fileName << std::endl;
            _width = 0;
            _height = 0;
        } else {
            std::cout << "Successfully loaded: " << fileName << " (" << _width << "x" << _height << ")" << std::endl;
        }
    }

    Graphic::Pixel Texture::sampleUVColor(double u, double v)
    {
        if (!this->_textureData)
            return {255, 0, 255};

        int x = static_cast<int>(u * (_width - 1));
        int y = static_cast<int>(v * (_height - 1));

        x = std::max(0, std::min(x, _width - 1));
        y = std::max(0, std::min(y, _height - 1));

        int index = (y * _width + x) * 3;

        return {
            _textureData[index],
            _textureData[index + 1],
            _textureData[index + 2]
        };
    }
} // namespace RayTracer
