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

            void addLight(const DirectionalLight &light) { _directionalLights.push_back(light); }
            void addPointLight(const PointLight &light) { _pointLights.push_back(light); }
            void setAmbientLight(const AmbientLight &light) { _ambientLight = light; }
            void loadHDRI(const std::string& path) { _hdriPath = path; _hdri = HDRI(path); }
            const std::string& getHDRIPath() const noexcept { return _hdriPath; }

            std::vector<DirectionalLight> &getDirectionalLights() noexcept { return _directionalLights; }
            const std::vector<DirectionalLight> &getDirectionalLights() const noexcept { return _directionalLights; }
            const std::vector<DirectionalLight> &getLights() const noexcept { return _directionalLights; }

            std::vector<PointLight> &getPointLights() noexcept { return _pointLights; }
            const std::vector<PointLight> &getPointLights() const noexcept { return _pointLights; }

            AmbientLight &getAmbientLight() noexcept { return _ambientLight; }
            const AmbientLight &getAmbientLight() const noexcept { return _ambientLight; }

            const Graphic::Pixel getHDRIPixel(const Math::Vector3D& v) const { return _hdri.getPixelColorFromVec(v); }
            float &getEnvIntensity() {return this->_hdri._envExposure;};

        private:
            std::vector<DirectionalLight> _directionalLights;
            std::vector<PointLight> _pointLights;
            AmbientLight _ambientLight;
            HDRI _hdri;
            std::string _hdriPath;
    };

}
#endif
