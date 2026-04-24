#include <string>
#include <cctype>
#include <filesystem>
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

    try {
        for (const auto& entry : std::filesystem::directory_iterator(target_path)) {
            if (std::filesystem::is_regular_file(entry.path())) {
                std::string filePath = entry.path().string();
                if (checkFileExtension(filePath, "so") == false)
                    continue;
                std::string libName = entry.path().stem().string();;
                try {
                    this->_loaders.emplace_back();
                    this->_loaders.back().loadNewLib(filePath);

                    IRayFeature *feature = this->_loaders.back().getInstance(entryPointSymbol);
                    this->_pipeline.push_back(std::unique_ptr<IRayFeature>(feature));

                } catch (std::runtime_error) {
                    continue;
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        throw IntegratorException("Failed to load ray features.");
    }
}
