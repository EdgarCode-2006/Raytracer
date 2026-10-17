#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include "Scene.hpp"
#include "integrators/IntegrationManager.hpp"
#include "renderer/Renderer.hpp"

#include <functional>
#include <memory>

namespace RayTracer {

    // Renders the scene, nothing else: whoever owns the main loop changes the scene, then calls
    // renderFrame. The frames are traced on the render threads, so the image shown can be a frame
    // or two behind the scene.
    class Engine {
        public:
            Engine(RayTracer::Scene& scene, std::unique_ptr<Graphic::IRenderer> renderer);
            ~Engine();

            bool windowOpen() { return _renderer->windowOpen(); }
            // snapshots the scene, starts tracing it and shows the newest finished frame with
            // drawOverlay's ImGui calls on top
            void renderFrame(const std::function<void()> &drawOverlay);

            // the shader features, to turn them on and off (read when a frame starts tracing)
            std::vector<std::unique_ptr<IRayFeature>> &getFeatures() noexcept { return _integrationManager->getIntegrator()->getPipeline(); }
            Graphic::IRenderer &getRenderer() noexcept { return *_renderer; }

        private:
            void generateSnapShot();

            Scene& _scene;

            std::unique_ptr<Graphic::IRenderer> _renderer;
            std::unique_ptr<RayTracer::IntegrationManager> _integrationManager;
            std::shared_ptr<Frame> _currentFrame;
    };

    class EngineException : public std::exception {
        public:
            EngineException(const std::string& message) : _message(message) {}
            const char* what() const noexcept override { return _message.c_str(); }
        private:
            std::string _message;
    };

}
#endif
