#ifndef HDRI_HPP_
#define HDRI_HPP_
#include <tinyexr.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cmath>
#include <tuple>
#include "utils/Pixel.hpp"
#include "utils/math.hpp"

namespace RayTracer {
    class HDRI {
        public:
            HDRI(const std::string &filePath);
            ~HDRI();
            void initHDRIFromFilePath(const std::string &filePath);
            Graphic::Pixel getPixelColorFromVec(const Math::Vector3D &vec) const;
            void setExposure(float exposure) { _exposure = exposure; }
            float getExposure() const { return _exposure; }
        private:
            int _getIndexFromUV(float u, float v) const;
            std::tuple<float, float> _getUVFromVec(const Math::Vector3D &vec) const;
            Graphic::Pixel _applyExposure(float r, float g, float b) const;

            float *_rgbBuffer = nullptr;
            Graphic::Pixel _defaultColor;
            int _width = 0;
            int _height = 0;
            int _size;
            float _exposure = 1.0f;
    };

}

#endif