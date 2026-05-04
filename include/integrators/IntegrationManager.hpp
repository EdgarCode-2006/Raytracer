#ifndef INTEGRATION_MANAGER_HPP_
#define INTEGRATION_MANAGER_HPP_
#include <memory>
#include "core/Scene.hpp"
#include "integrators/Integrator.hpp"

namespace RayTracer {
    class IntegrationManager {
        public:
            IntegrationManager(std::shared_ptr<const RenderScene> scene);
            ~IntegrationManager() = default;
            void renderSnapShot(const Camera &cam);
            void addSnapShot(std::shared_ptr<const RenderScene> scene) {
                _snapShotQueue.push(scene);
            };
            
            std::shared_ptr<Frame> getAvailableFrame() {return this->_integrator->getAvailableFrame();};
            void stop() {this->_integrator->stop();};
            void instanceTiles(const Camera &cam) {this->_integrator->instanceTiles(cam);};
            void waitForRendering() {this->_integrator->waitForRendering();};
        private:
            std::unique_ptr<Integrator> _integrator;
            std::queue<std::shared_ptr<const RenderScene>> _snapShotQueue;
    };
}

#endif