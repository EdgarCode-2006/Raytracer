#ifndef LIGHTS_HPP_
#define LIGHTS_HPP_

#include "entities/Light.hpp"
#include "entities/HDRI.hpp"
#include <vector>

namespace RayTracer {

    class Lights {
        public:
            Lights() = default;
            ~Lights() = default;
            
            Lights(const Lights &) = delete;
            Lights &operator=(const Lights &) = delete;
            Lights(Lights &&) = default;
            Lights &operator=(Lights &&) = default;

            void addLight(const DirectionalLight &light) {_directionalLights.push_back(light);}
            void setAmbientLight(const AmbientLight &light) {_ambientLight = light;}
            void loadHDRI(const std::string& path) {_hdri = HDRI(path);}

            const std::vector<DirectionalLight> &getLights() const noexcept {return _directionalLights;}
            const AmbientLight &getAmbientLight() const noexcept {return _ambientLight;}

            const Graphic::Pixel getHDRIPixel(const Math::Vector3D& v) const {return _hdri.getPixelColorFromVec(v);}

        private:
            std::vector<DirectionalLight> _directionalLights;
            AmbientLight _ambientLight;
            HDRI _hdri;
        };

}
#endif