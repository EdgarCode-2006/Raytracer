#include "OBJParser.hpp"
#include "utils/Material.hpp"
#include "Engine.hpp"
#include "Scene.hpp"
#include <iostream>
#include "renderer/RaylibRenderer.hpp"
#include <memory>
#include <iostream>
#include <raylib.h>


static void positionCamera(RayTracer::Scene& scene, const std::vector<RayTracer::Triangle>& triangles)
{
    double minx=1e9, maxx=-1e9, miny=1e9, maxy=-1e9, minz=1e9, maxz=-1e9;

    for (const auto& tri : triangles) {
        for (const auto* v : {&tri.a(), &tri.b(), &tri.c()}) {
            minx = std::min(minx, v->x()); maxx = std::max(maxx, v->x());
            miny = std::min(miny, v->y()); maxy = std::max(maxy, v->y());
            minz = std::min(minz, v->z()); maxz = std::max(maxz, v->z());
        }
    }

    Math::Vector3D center((minx+maxx)/2, (miny+maxy)/2, (minz+maxz)/2);
    double size = std::max(maxx-minx, std::max(maxy-miny, maxz-minz));

    scene.getCamera().setPosition(
        Math::Vector3D(center.x(), center.y(), center.z() - size * 1.5)
    );
}

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

        const int screenWidth  = 800;
        const int screenHeight = 600;

        if (triangles.empty()) {
            std::cerr << "Error: no triangles loaded from " << argv[1] << std::endl;
            return 84;
        }
        RayTracer::Scene scene(screenWidth, screenHeight);
        scene.reserve(1, triangles.size(), 2);
        try {
            scene.loadHDRI("assets/HDRIs/HDRI.exr");
        } catch (const std::exception& e) {
            std::cerr << "HDRI load failed: " << e.what() << '\n';
        }
        uint32_t meshId = scene.addMesh(std::move(triangles));
        Graphic::Material mat;
        mat.color = Graphic::Pixel(200, 230, 255, 255);
        mat.ior = 1.33;
        mat.refractivity = 0.69;
        mat.reflectivity = 0.4;
        scene.addInstance(meshId, Math::Matrix4{}, mat);
        scene.addInstance(meshId, Math::Matrix4{}, mat);
        scene.buildTLAS();
        scene.getCamera().fitToScene(scene.getGeometry().getAllTriangles());

        RayTracer::AmbientLight ambient;
        ambient.intensity = 0.2;
        scene.setAmbientLight(ambient);

        RayTracer::DirectionalLight light;
        // RayTracer::DirectionalLight light2;
        light.direction = Math::Vector3D(-1.2, 0.1, 0.1).normalize();
        light.intensity = 0.8;
        // light2.direction = Math::Vector3D(-1.5, -0.1, 4).normalize();
        // light2.intensity = 0.7;
        scene.addLight(light);
        // scene.addLight(light2);

        auto renderer = std::make_unique<Graphic::RaylibRenderer>(screenWidth, screenHeight);

        RayTracer::Engine engine(scene, std::move(renderer));
        engine.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }

    return 0;
}
