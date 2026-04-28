#include "entities/HDRI.hpp"

namespace RayTracer
{
    HDRI::HDRI(const std::string &filePath) {
        _defaultColor = {130, 2, 50};
        this->initHDRIFromFilePath(filePath);
    }

    HDRI::~HDRI() {
        free(this->_rgbBuffer);
    }

    void HDRI::initHDRIFromFilePath(const std::string &filePath)
    {
        if (this->_rgbBuffer)
            free(this->_rgbBuffer);
        const char* err = nullptr;

        int ret = LoadEXR(&_rgbBuffer, &_width, &_height, filePath.c_str(), &err);

        if (ret != TINYEXR_SUCCESS) {
            if (err) {
                fprintf(stderr, "Detailed Error: %s\n", err);
                FreeEXRErrorMessage(err);
            }
            return;
        }
        this->_size = _width * _height;
    }

    Graphic::Pixel HDRI::_applyExposure(float r, float g, float b) const
    {
        r = r * ENV_EXPOSURE;
        g = g * ENV_EXPOSURE;
        b = b * ENV_EXPOSURE;

        r = r / (r + 1.0f);
        g = g / (g + 1.0f);
        b = b / (b + 1.0f);

        r = r * 255.0f;
        g = g * 255.0f;
        b = b * 255.0f;

        return Graphic::Pixel(r, g, b, 255);
    }

    Graphic::Pixel HDRI::getPixelColorFromVec(const Math::Vector3D &vec) const
    {
        if (!this->_rgbBuffer)
            return this->_defaultColor;

        std::tuple<float, float> uv = this->_getUVFromVec(vec);
        int id = this->_getIndexFromUV(std::get<0>(uv), std::get<1>(uv));

        if (id < 0 || id + 3 >= _width * _height * 4)
            return this->_defaultColor;

        float r = _rgbBuffer[id + 0];
        float g = _rgbBuffer[id + 1];
        float b = _rgbBuffer[id + 2];
    
        return this->_applyExposure(r, g, b);
    }

    int HDRI::_getIndexFromUV(float u, float v) const
    {
        int x = static_cast<int>(u * (this->_width - 1));
        int y = static_cast<int>(v * (this->_height - 1));

        return (y * this->_width + x) * 4;   
    }

    std::tuple<float, float> HDRI::_getUVFromVec(const Math::Vector3D &vec) const
    {
        float u = (1.0f / (2.0f * M_PI)) * atan2(vec.z(), vec.x()) + 0.5;
        float v = (1.0f / M_PI) * asin(vec.y()) + 0.5;

        return std::tuple<float, float>(u, v);
    }

    
}
