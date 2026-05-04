#ifndef IRENDERER_HPP_
#define IRENDERER_HPP_
#include "utils/Pixel.hpp"
#include "core/Scene.hpp"
#include "entities/Camera.hpp"
#include <iostream>
#include <vector>
#include <functional>
#include <memory>

namespace RayTracer { class Integrator; }

namespace Graphic {
    class IRenderer {
        public:
            virtual ~IRenderer() = default;
            virtual void resizeWindow(int newWidth, int newHeight) = 0;
            virtual void pixelsToScreenBuffer(Pixel *pixels) = 0;
            virtual void renderPixelBuffer(Pixel *pixels, RayTracer::Scene& scene, RayTracer::Camera& cam) = 0;
            virtual bool windowOpen() = 0;
            std::function<void()> onSnapshotRequested;
            virtual void setIntegrator(RayTracer::Integrator*) noexcept {}

    };

    class ARenderer : public IRenderer {
        public:
            virtual ~ARenderer() = default;
            virtual void resizeWindow(int newWidth, int newHeight) = 0;
            virtual void pixelsToScreenBuffer(Pixel *pixels) = 0;
            virtual void renderPixelBuffer(Pixel *pixels, RayTracer::Scene& scene, RayTracer::Camera& cam) = 0;
            virtual bool windowOpen() = 0;
        protected:
            std::vector<Pixel> _pixels;
    };

    class RendererException : public std::exception {
        public:
            RendererException(const std::string& message) : _message(message) {}
            const char* what() const noexcept override { return _message.c_str(); }
        private:
            std::string _message;
    };
}
#endif