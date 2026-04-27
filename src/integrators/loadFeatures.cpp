#include <string>
#include <cctype>
#include <filesystem>
#include <unordered_set>
#include "integrators/Integrator.hpp"

static bool checkFileExtension(const std::string& filename, const std::string& extension)
{
    size_t dotPosition = filename.find_last_of('.');
    if (dotPosition != std::string::npos) {
        return filename.substr(dotPosition + 1) == extension;
    }
    return false;
}

void RayTracer::Integrator::loadRayFeatures()
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

        std::cout << "feature count = " << _pipeline.size() << std::endl;
        // this->setPluginState("SpecularHandler", false);
        this->setPluginState("AmbientOcclusion", false);
    } catch (const std::filesystem::filesystem_error& ex) {
        throw IntegratorException("Failed to load ray features.");
    }
}
