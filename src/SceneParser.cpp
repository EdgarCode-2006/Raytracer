#include "SceneParser.hpp"
#include "io/OBJParser.hpp"
#include "utils/Material.hpp"
#include <libconfig.h++>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace RayTracer {

static void loadObjects(const libconfig::Setting& objects, Scene& scene)
{
    int n = objects.getLength();
    std::vector<std::vector<Triangle>> allTris(n);
    uint32_t totalTris = 0;

    for (int i = 0; i < n; i++) {
        std::string objPath = (const char*)objects[i]["obj"];
        OBJParser::OBJParser parser;
        allTris[i] = parser.load(objPath);
        if (allTris[i].empty())
            throw std::runtime_error("Failed to load OBJ: " + objPath);
        totalTris += (uint32_t)allTris[i].size();
    }

    uint32_t existingMeshes = scene.getGeometry().getMeshCount();
    uint32_t existingTris = (uint32_t)scene.getGeometry().getAllTriangles().size();
    uint32_t existingInstances = scene.getInstanceCount();
    scene.reserve(existingMeshes + n, existingTris + totalTris, existingInstances + n);

    for (int i = 0; i < n; i++) {
        const libconfig::Setting& obj = objects[i];
        std::string objPath = (const char*)obj["obj"];

        uint32_t meshId = scene.addMesh(std::move(allTris[i]), objPath);

        Graphic::Material mat;
        if (obj.exists("color")) {
            const libconfig::Setting& c = obj["color"];
            mat.color = Graphic::Pixel(
                (unsigned char)(int)c["r"],
                (unsigned char)(int)c["g"],
                (unsigned char)(int)c["b"]
            );
        }
        if (obj.exists("reflectivity")) mat.reflectivity = (double)obj["reflectivity"];
        if (obj.exists("refractivity")) mat.refractivity = (double)obj["refractivity"];
        if (obj.exists("ior")) mat.ior = (double)obj["ior"];

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

        Instance& inst = scene.addInstance(meshId, Math::Matrix4{}, mat);
        inst.setTRS(pos, rot, scale);
    }
}

void SceneParser::load(const std::string& path, Scene& scene)
{
    libconfig::Config cfg;

    try {
        cfg.readFile(path.c_str());
    } catch (const libconfig::FileIOException&) {
        throw std::runtime_error("Cannot open scene file: " + path);
    } catch (const libconfig::ParseException& e) {
        throw std::runtime_error(std::string("Parse error in ") + e.getFile()
            + " line " + std::to_string(e.getLine()) + ": " + e.getError());
    }

    if (cfg.exists("imports")) {
        const libconfig::Setting& imports = cfg.lookup("imports");
        for (int i = 0; i < imports.getLength(); i++) {
            std::string importPath = (const char*)imports[i];
            load(importPath, scene);
        }
    }

    if (cfg.exists("hdri")) {
        std::string hdriPath = (const char*)cfg.lookup("hdri");
        scene.loadHDRI(hdriPath);
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
        if (cam.exists("yaw") || cam.exists("pitch"))
            scene.getCamera().setOrientation(
                cam.exists("yaw") ? (double)cam["yaw"] : 0.0,
                cam.exists("pitch") ? (double)cam["pitch"] : 0.0
            );
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
        if (lights.exists("point")) {
            const libconfig::Setting& pts = lights["point"];
            for (int i = 0; i < pts.getLength(); i++) {
                const libconfig::Setting& p = pts[i];
                PointLight pl;
                pl.position = Math::Vector3D(
                    (double)p["position"]["x"],
                    (double)p["position"]["y"],
                    (double)p["position"]["z"]
                );
                pl.intensity = p.exists("intensity") ? (double)p["intensity"] : 1.0;
                scene.addPointLight(pl);
            }
        }
    }

    if (cfg.exists("objects"))
        loadObjects(cfg.lookup("objects"), scene);

    if (cfg.exists("features")) {
        const libconfig::Setting& feats = cfg.lookup("features");
        for (int i = 0; i < feats.getLength(); i++) {
            std::string name = (const char*)feats[i]["name"];
            bool active = (bool)feats[i]["active"];
            scene.setSavedFeature(name, active);
        }
    }

    scene.buildTLAS();
}

void SceneParser::save(const std::string& path, const Scene& scene,
    const std::vector<std::pair<std::string, bool>>& features)
{
    libconfig::Config cfg;
    libconfig::Setting& root = cfg.getRoot();

    auto& cam = root.add("camera", libconfig::Setting::TypeGroup);
    auto& camPos = cam.add("position", libconfig::Setting::TypeGroup);
    auto camP = scene.getCamera().getPosition();
    camPos.add("x", libconfig::Setting::TypeFloat) = camP.x();
    camPos.add("y", libconfig::Setting::TypeFloat) = camP.y();
    camPos.add("z", libconfig::Setting::TypeFloat) = camP.z();
    cam.add("fieldOfView", libconfig::Setting::TypeFloat) = scene.getCamera().getFov();
    cam.add("yaw", libconfig::Setting::TypeFloat) = scene.getCamera().getYaw();
    cam.add("pitch", libconfig::Setting::TypeFloat) = scene.getCamera().getPitch();

    auto& lights = root.add("lights", libconfig::Setting::TypeGroup);
    lights.add("ambient", libconfig::Setting::TypeFloat) = scene.getAmbientLight().intensity;
    auto& dirs = lights.add("directional", libconfig::Setting::TypeList);
    for (const auto& dl : scene.getDirectionalLights()) {
        auto& d = dirs.add(libconfig::Setting::TypeGroup);
        auto& dir = d.add("direction", libconfig::Setting::TypeGroup);
        dir.add("x", libconfig::Setting::TypeFloat) = dl.direction.x();
        dir.add("y", libconfig::Setting::TypeFloat) = dl.direction.y();
        dir.add("z", libconfig::Setting::TypeFloat) = dl.direction.z();
        d.add("intensity", libconfig::Setting::TypeFloat) = dl.intensity;
    }
    auto& pts = lights.add("point", libconfig::Setting::TypeList);
    for (const auto& pl : scene.getPointLights()) {
        auto& p = pts.add(libconfig::Setting::TypeGroup);
        auto& pos = p.add("position", libconfig::Setting::TypeGroup);
        pos.add("x", libconfig::Setting::TypeFloat) = pl.position.x();
        pos.add("y", libconfig::Setting::TypeFloat) = pl.position.y();
        pos.add("z", libconfig::Setting::TypeFloat) = pl.position.z();
        p.add("intensity", libconfig::Setting::TypeFloat) = pl.intensity;
    }

    auto& objects = root.add("objects", libconfig::Setting::TypeList);
    for (uint32_t i = 0; i < scene.getInstanceCount(); i++) {
        const Instance& inst = scene.getInstance(i);
        const Graphic::Material& mat = inst.getMaterial();
        const float* p = inst.getPos();
        const float* r = inst.getRot();
        const float* s = inst.getScale();

        auto& obj = objects.add(libconfig::Setting::TypeGroup);
        obj.add("obj", libconfig::Setting::TypeString) = scene.getMeshPath(inst.getMeshId());

        auto& pos = obj.add("position", libconfig::Setting::TypeGroup);
        pos.add("x", libconfig::Setting::TypeFloat) = (double)p[0];
        pos.add("y", libconfig::Setting::TypeFloat) = (double)p[1];
        pos.add("z", libconfig::Setting::TypeFloat) = (double)p[2];

        auto& rot = obj.add("rotation", libconfig::Setting::TypeGroup);
        rot.add("x", libconfig::Setting::TypeFloat) = (double)r[0];
        rot.add("y", libconfig::Setting::TypeFloat) = (double)r[1];
        rot.add("z", libconfig::Setting::TypeFloat) = (double)r[2];

        auto& sc = obj.add("scale", libconfig::Setting::TypeGroup);
        sc.add("x", libconfig::Setting::TypeFloat) = (double)s[0];
        sc.add("y", libconfig::Setting::TypeFloat) = (double)s[1];
        sc.add("z", libconfig::Setting::TypeFloat) = (double)s[2];

        auto& color = obj.add("color", libconfig::Setting::TypeGroup);
        color.add("r", libconfig::Setting::TypeInt) = (int)mat.color._r;
        color.add("g", libconfig::Setting::TypeInt) = (int)mat.color._g;
        color.add("b", libconfig::Setting::TypeInt) = (int)mat.color._b;

        obj.add("reflectivity", libconfig::Setting::TypeFloat) = (double)mat.reflectivity;
        obj.add("refractivity", libconfig::Setting::TypeFloat) = (double)mat.refractivity;
        obj.add("ior", libconfig::Setting::TypeFloat) = (double)mat.ior;
    }

    if (!scene.getHDRIPath().empty())
        root.add("hdri", libconfig::Setting::TypeString) = scene.getHDRIPath();

    if (!features.empty()) {
        auto& feats = root.add("features", libconfig::Setting::TypeList);
        for (const auto& [name, active] : features) {
            auto& f = feats.add(libconfig::Setting::TypeGroup);
            f.add("name", libconfig::Setting::TypeString) = name;
            f.add("active", libconfig::Setting::TypeBoolean) = active;
        }
    }

    try {
        cfg.writeFile(path.c_str());
    } catch (const libconfig::FileIOException&) {
        throw std::runtime_error("Cannot write scene file: " + path);
    }
}

}
