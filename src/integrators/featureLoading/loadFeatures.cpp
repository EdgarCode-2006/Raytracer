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
    this->refreshActiveFeatures();
}

void RayTracer::PipelineManager::refreshActiveFeatures()
{
    for (auto &features : _activeFeatures)
        features.clear();

    for (const auto &[type, indices] : _shaderMap) {
        if (type < 0 || type >= SHADER_TYPE_COUNT)
            continue;
        for (int i : indices)
            if (_pipeline[i]->isActive())
                _activeFeatures[type].push_back(_pipeline[i].get());
    }
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
                if (checkFileExtension(filePath, DynamicLibrary::EXTENSION) == false)
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

                } catch (const std::runtime_error &e) {
                    std::cerr << "couldn't load feature " << filePath << ": " << e.what() << std::endl;
                    continue;
                }
            }
        }

        // std::sort(_pipeline.begin(), _pipeline.end(), 
        //     [](const std::unique_ptr<IRayFeature>& a, const std::unique_ptr<IRayFeature>& b) {
        //         return a->getType() < b->getType();
        //     }
        // );
        for (int i = 0; i < this->_pipeline.size(); i++) {
            this->_shaderMap[this->_pipeline[i]->getType()].push_back(i);
        }
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
