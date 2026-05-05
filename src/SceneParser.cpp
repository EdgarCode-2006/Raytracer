#include "SceneParser.hpp"
#include "OBJParser.hpp"
#include "utils/Material.hpp"
#include <libconfig.h++>
#include <stdexcept>

namespace RayTracer {

void SceneParser::load(const std::string& path, Scene& scene)
{
    libconfig::Config cfg;

    try {
        cfg.readFile(path.c_str());
    } catch (const libconfig::FileIOException&) {
        throw std::runtime_error("Cannot open scene file: " + path);
    } catch (const libconfig::ParseException& e) {
        throw std::runtime_error(std::string("Parse error in ") + e.getFile() + " line " + std::to_string(e.getLine()) + ": " + e.getError());
    }

    if (cfg.exists("camera")) {
        const libconfig::Setting& cam = cfg.lookup("camera");

        if (cam.exists("position")) {
            const libconfig::Setting& pos = cam["position"];
            scene.getCamera().setPosition(Math::Vector3D(
                (double)pos["x"], (double)pos["y"], (double)pos["z"]
            ));
        }
        if (cam.exists("fieldOfView"))
            scene.getCamera().setFov((double)cam["fieldOfView"]);
    }

    if (cfg.exists("lights")) {
        const libconfig::Setting& lights = cfg.lookup("lights");

        if (lights.exists("ambient")) {
            AmbientLight al;
            al.intensity = (double)lights["ambient"];
            scene.setAmbientLight(al);
        }
        if (lights.exists("directional")) {
            const libconfig::Setting& dirs = lights["directional"];
            for (int i = 0; i < dirs.getLength(); i++) {
                const libconfig::Setting& d = dirs[i];
                DirectionalLight dl;
                dl.direction = Math::Vector3D(
                    (double)d["direction"]["x"],
                    (double)d["direction"]["y"],
                    (double)d["direction"]["z"]
                ).normalize();
                dl.intensity = d.exists("intensity") ? (double)d["intensity"] : 1.0;
                scene.addLight(dl);
            }
        }
    }

    if (!cfg.exists("objects"))
        return;

    const libconfig::Setting& objects = cfg.lookup("objects");

    scene.reserveMeshes(objects.getLength(), 0, objects.getLength());

    for (int i = 0; i < objects.getLength(); i++) {
        const libconfig::Setting& obj = objects[i];

        std::string objPath = (const char*)obj["obj"];

        OBJParser::OBJParser parser;
        std::vector<Triangle> tris = parser.load(objPath);
        if (tris.empty())
            throw std::runtime_error("Failed to load OBJ: " + objPath);

        uint32_t meshId = scene.addMesh(std::move(tris));

        Graphic::Material mat;
        if (obj.exists("color")) {
            const libconfig::Setting& c = obj["color"];
            mat.color = Graphic::Pixel(
                (unsigned char)(int)c["r"],
                (unsigned char)(int)c["g"],
                (unsigned char)(int)c["b"]
            );
        }

        float pos[3] = {0, 0, 0};
        float rot[3] = {0, 0, 0};
        float scale[3] = {1, 1, 1};

        if (obj.exists("position")) {
            pos[0] = (double)obj["position"]["x"];
            pos[1] = (double)obj["position"]["y"];
            pos[2] = (double)obj["position"]["z"];
        }
        if (obj.exists("rotation")) {
            rot[0] = (double)obj["rotation"]["x"];
            rot[1] = (double)obj["rotation"]["y"];
            rot[2] = (double)obj["rotation"]["z"];
        }
        if (obj.exists("scale")) {
            scale[0] = (double)obj["scale"]["x"];
            scale[1] = (double)obj["scale"]["y"];
            scale[2] = (double)obj["scale"]["z"];
        }

        Instance& inst = scene.addInstance(meshId, Matrix4{}, mat);
        inst.setTRS(pos, rot, scale);
    }

    scene.buildTLAS();
}

}
