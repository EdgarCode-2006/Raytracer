#ifndef PIPELINE_MANAGER_HPP_
#define PIPELINE_MANAGER_HPP_
#include <array>
#include <vector>
#include <memory>
#include "integrators/rayFeature/IRayFeature.hpp"
#include "integrators/featureLoader/DLLoader.hpp"

namespace RayTracer {
    inline constexpr int SHADER_TYPE_COUNT = DEBUG + 1;

    class IRayFeature;
    class PipelineManager {
        public:
            PipelineManager();
            ~PipelineManager() = default;
            void loadRayFeatures();
            void setPluginState(const std::string &pluginName, const bool &state);

            // Rebuilds the active features lists, to call when no frame is being rendered:
            // features switched on or off since then only change the next frame.
            void refreshActiveFeatures();
            const std::vector<IRayFeature *> &getActiveFeatures(ShaderType type) const noexcept {
                return _activeFeatures[type];
            }
            const std::array<std::vector<IRayFeature *>, SHADER_TYPE_COUNT> &getActiveFeatures() const noexcept {
                return _activeFeatures;
            }

            std::vector<std::unique_ptr<IRayFeature>> _pipeline;
            std::unordered_map<ShaderType, std::vector<int>> _shaderMap;
        private:
            std::vector<DLLoader<IRayFeature>> _loaders;
            // active features of each shader type, in pipeline order
            std::array<std::vector<IRayFeature *>, SHADER_TYPE_COUNT> _activeFeatures;
    };
}
#endif
