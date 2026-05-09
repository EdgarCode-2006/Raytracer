#ifndef PIPELINE_MANAGER_HPP_
#define PIPELINE_MANAGER_HPP_
#include <vector>
#include <memory>
#include "integrators/rayFeature/IRayFeature.hpp"
#include "integrators/featureLoader/DLLoader.hpp"

namespace RayTracer {
    class IRayFeature;
    class PipelineManager {
        public:
            PipelineManager();
            ~PipelineManager() = default;
            void loadRayFeatures();
            void setPluginState(const std::string &pluginName, const bool &state);

            std::vector<std::unique_ptr<IRayFeature>> _pipeline;
            std::unordered_map<ShaderType, std::vector<int>> _shaderMap;
        private:
            std::vector<DLLoader<IRayFeature>> _loaders;
    };
}
#endif
