#include "OBJParser.hpp"
#include <iostream>
#include "Engine.hpp"
#include "Scene.hpp"

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.obj>" << std::endl;
        return 1;
    }

    try {
        OBJParser::OBJParser parser;
        std::vector<RayTracer::Triangle> triangles = parser.load(argv[1]);
        const int screenWidth = 800;
        const int screenHeight = 450;

        RayTracer::Scene scene(screenWidth, screenHeight);
        scene.addTriangles(triangles);

        RayTracer::Engine engine(scene);
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
