#include "OBJParser.hpp"
#include "Engine.hpp"
#include "Scene.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.obj>" << std::endl;
        return 84;
    }

    try {
        OBJParser::OBJParser parser;
        std::vector<RayTracer::Triangle> triangles = parser.load(argv[1]);

        if (triangles.empty()) {
            std::cerr << "Error: no triangles loaded from " << argv[1] << std::endl;
            return 84;
        }

        const int screenWidth  = 200;
        const int screenHeight = 112;

        RayTracer::Scene scene(screenWidth, screenHeight);
        scene.addTriangles(triangles);
        scene.getCamera().fitToScene(triangles);

        RayTracer::AmbientLight ambient;
        ambient.intensity = 0.2;
        scene.setAmbientLight(ambient);

        RayTracer::DirectionalLight light;
        light.direction = Math::Vector3D(1, 2, -1).normalize();
        light.intensity = 0.8;
        scene.addLight(light);

        auto& cam = scene.getCamera();
        std::cerr << "triangles: " << triangles.size() << std::endl;
        std::cerr << "cam pos: " << cam.getPosition().x << " "
                  << cam.getPosition().y << " "
                  << cam.getPosition().z << std::endl;
        std::cerr << "cam fov: " << cam.getFov() << std::endl;

        RayTracer::Engine engine(scene);
        engine.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }

    return 0;
}
