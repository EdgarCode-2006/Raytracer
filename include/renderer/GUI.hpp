#ifndef GUI_HPP_
#define GUI_HPP_

#include "core/Scene.hpp"
#include "entities/Camera.hpp"
#include <array>
#include <vector>

namespace RayTracer { class Integrator; }

namespace Graphic {

    class GUI {
        public:
            void Render(RayTracer::Scene& scene, RayTracer::Camera& cam);
            bool wasSceneModified() const { return _sceneIsModified; }
            void setIntegrator(RayTracer::Integrator* integrator) noexcept { _integrator = integrator; }

        private:
            void renderCamera(RayTracer::Camera& cam);
            void renderObjects(RayTracer::Scene& scene);
            void renderLights(RayTracer::Scene& scene);
            void renderFeatures();
            void renderScene(RayTracer::Scene& scene);

            bool _sceneIsModified = false;
            RayTracer::Integrator* _integrator = nullptr;
            char _savePath[256] = "assets/scenes/saved.cfg";
            char _hdriPath[256] = "assets/HDRIs/";
            int _selectedLightType = 0;
            std::vector<std::array<char, 256>> _texturePaths;
    };

}

#endif
