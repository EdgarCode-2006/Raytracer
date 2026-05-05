#include "io/OBJParser.hpp"
#include "utils/Material.hpp"
#include "core/Engine.hpp"
#include "core/Scene.hpp"
#include "SceneParser.hpp"
#include <iostream>
#include "renderer/RaylibRenderer.hpp"
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <scene.cfg|file.obj>" << std::endl;
        return 84;
    }

    const int screenWidth = 800;
    const int screenHeight = 600;

    try {
        RayTracer::Scene scene(screenWidth, screenHeight);
        std::string arg = argv[1];

        if (arg.size() >= 4 && arg.substr(arg.size() - 4) == ".cfg") {
            RayTracer::SceneParser parser;
            parser.load(arg, scene);
            if (scene.getGeometry().getAllTriangles().empty()) {
                std::cerr << "Error: no objects loaded from " << arg << std::endl;
                return 84;
            }
        } else {
            OBJParser::OBJParser parser;
            std::vector<RayTracer::Triangle> tris = parser.load(arg);
            if (tris.empty()) {
                std::cerr << "Error: no triangles loaded from " << arg << std::endl;
                return 84;
            }
            scene.reserve(1, tris.size(), 1);
            uint32_t meshId = scene.addMesh(std::move(tris));
            scene.addInstance(meshId);
            scene.buildTLAS();

            RayTracer::AmbientLight ambient;
            ambient.intensity = 0.2;
            scene.setAmbientLight(ambient);

            RayTracer::DirectionalLight light;
            light.direction = Math::Vector3D(-1.2, 0.1, 0.1).normalize();
            light.intensity = 0.8;
            scene.addLight(light);
        }

        scene.getCamera().fitToScene(scene.getGeometry().getAllTriangles());

        auto renderer = std::make_unique<Graphic::RaylibRenderer>(screenWidth, screenHeight);
        RayTracer::Engine engine(scene, std::move(renderer));
        engine.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }

    return 0;
}
