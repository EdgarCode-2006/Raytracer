#include "OBJParser.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.obj>" << std::endl;
        return 1;
    }

    OBJParser::OBJParser parser;
    std::vector<RayTracer::Triangle> triangles = parser.load(argv[1]);

    std::cout << triangles.size() << " triangles loaded" << std::endl;
    std::cout << "Triangle " << 1 << ": "
        << "(" << triangles[1].a.x << "," << triangles[1].a.y << "," << triangles[1].a.z << ") "
        << "(" << triangles[1].b.x << "," << triangles[1].b.y << "," << triangles[1].b.z << ") "
        << "(" << triangles[1].c.x << "," << triangles[1].c.y << "," << triangles[1].c.z << ")"
        << std::endl;
    return 0;
}
