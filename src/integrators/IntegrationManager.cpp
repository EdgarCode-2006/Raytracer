#include "integrators/IntegrationManager.hpp"

namespace RayTracer
{
    IntegrationManager::IntegrationManager(std::shared_ptr<const RenderScene> scene)
    {
        // create the Integrator with the current scene snapshot.
        this->_integrator = std::make_unique<Integrator>(scene);
    }

    void IntegrationManager::renderSnapShot(const Camera &cam)
    {
        bool newFrame = false;

        if (!this->_snapShotQueue.empty()) {
            std::shared_ptr<const RenderScene> currentSnapShot = this->_snapShotQueue.front();
            this->_snapShotQueue.pop();
            this->_integrator->setCurrentSceneSnapshot(currentSnapShot);
        }

        this->_integrator->renderScene(cam);
    }
} // namespace RayTracer
