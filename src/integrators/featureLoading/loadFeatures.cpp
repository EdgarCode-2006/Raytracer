#include <string>
#include <cctype>
#include <filesystem>
#include <unordered_set>
#include "integrators/Integrator.hpp"
#include "integrators/featureLoader/PipelineManager.hpp"

RayTracer::PipelineManager::PipelineManager()
{
    try {
        this->loadRayFeatures();
    } catch (const IntegratorException &e) {
        throw IntegratorException(e.what());
    }

    std::cout << "feature count = " << _pipeline.size() << std::endl;
    this->setPluginState("ShadeSmooth", false);
    this->setPluginState("AmbientOcclusion", false);
    this->setPluginState("PathTracing", false);
}

static bool checkFileExtension(const std::string& filename, const std::string& extension)
{
    size_t dotPosition = filename.find_last_of('.');
    if (dotPosition != std::string::npos) {
        return filename.substr(dotPosition + 1) == extension;
    }
    return false;
}

void RayTracer::PipelineManager::loadRayFeatures()
{
    const std::string entryPointSymbol = "getInstance";
    std::filesystem::path target_path{"./rayFeatures/"};
    std::unordered_set<std::string> loadedLibs;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(target_path)) {
            if (std::filesystem::is_regular_file(entry.path())) {
                std::string filePath = entry.path().string();
                if (checkFileExtension(filePath, "so") == false)
                    continue;
                std::string libName = entry.path().stem().string();
                
                // Skip if already loaded
                if (loadedLibs.count(libName) > 0)
                    continue;
                loadedLibs.insert(libName);
                
                try {
                    this->_loaders.emplace_back();
                    this->_loaders.back().loadNewLib(filePath);

                    IRayFeature *feature = this->_loaders.back().getInstance(entryPointSymbol);
                    this->_pipeline.push_back(std::unique_ptr<IRayFeature>(feature));
                    std::cout << "feature-> " << feature->getName() << std::endl;

                } catch (std::runtime_error) {
                    continue;
                }
            }
        }

        std::sort(_pipeline.begin(), _pipeline.end(), 
            [](const std::unique_ptr<IRayFeature>& a, const std::unique_ptr<IRayFeature>& b) {
                return a->getPriority() < b->getPriority();
            }
        );
        const ShaderType allShaderTypes[] = { EMISSION, DIFFUSION, CAUSTIC, UTILITY, DEBUG };

        for (size_t i = 0; i < this->_pipeline.size(); i++) {
            int featureMask = this->_pipeline[i]->getType();

            for (ShaderType type : allShaderTypes) {
                if ((featureMask & type) != 0) {
                    this->_shaderMap[type].push_back(i);
                }
            }
        }

        std::cout << "\n--- Shader Type Map Layout ---" << std::endl;
        for (const auto& [shaderTypeKey, featureIndices] : this->_shaderMap) {
            
            std::string typeName;
            switch (shaderTypeKey) {
                case EMISSION:  typeName = "EMISSION";  break;
                case DIFFUSION: typeName = "DIFFUSION"; break;
                case CAUSTIC:   typeName = "CAUSTIC";   break;
                case UTILITY:   typeName = "UTILITY";   break;
                case DEBUG:     typeName = "DEBUG";     break;
                default:        typeName = "UNKNOWN (" + std::to_string(shaderTypeKey) + ")"; break;
            }

            std::cout << "Stage [" << typeName << "] holds " << featureIndices.size() << " feature(s):" << std::endl;
            
            if (featureIndices.empty()) {
                std::cout << "  -> (empty)" << std::endl;
            } else {
                for (int idx : featureIndices) {
                    std::string featureName = this->_pipeline[idx]->getName();
                    std::cout << "  -> Index " << idx << ": " << featureName << std::endl;
                }
            }
            std::cout << "----------------------------------" << std::endl;
        }
        std::cout << std::endl;
    } catch (const std::filesystem::filesystem_error& ex) {
        throw IntegratorException("Failed to load ray features.");
    }
}

void RayTracer::PipelineManager::setPluginState(const std::string &pluginName, const bool &state)
{
    for (auto& plugin : _pipeline) {
        if (plugin->getName() == pluginName) {
            plugin->setActivity(state);
            return;
        }
    }
    std::cout << "plugin not found couldn't disable." << std::endl;
}
