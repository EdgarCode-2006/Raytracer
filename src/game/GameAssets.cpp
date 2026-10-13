#include "game/GameAssets.hpp"
#include "io/OBJParser.hpp"
#include <libconfig.h++>
#include <cmath>
#include <stdexcept>

namespace Game {

    void Model::addTo(RayTracer::Scene &scene, const Transform &transform) const
    {
        const float pos[3] = {(float)transform.position.x(), (float)transform.position.y(), (float)transform.position.z()};
        const float rot[3] = {(float)transform.rotation.x(), (float)transform.rotation.y(), (float)transform.rotation.z()};
        const float size[3] = {(float)scale, (float)scale, (float)scale};

        for (const auto &part : parts) {
            RayTracer::Instance &instance = scene.addInstance(part.meshId, Math::Matrix4{}, part.material);
            instance.setTRS(pos, rot, size);
        }
    }

    static double readDouble(const libconfig::Setting &setting, const char *name, double fallback)
    {
        return setting.exists(name) ? (double)setting[name] : fallback;
    }

    static Graphic::Material readMaterial(const libconfig::Setting &part)
    {
        Graphic::Material material;
        if (part.exists("color")) {
            const libconfig::Setting &c = part["color"];
            material.color = Graphic::Pixel((unsigned char)(int)c["r"], (unsigned char)(int)c["g"], (unsigned char)(int)c["b"]);
        }
        material.emission = (float)readDouble(part, "emission", 0.0);
        // a glowing part is not lit, it only shows its own color
        if (material.emission > 0.0f)
            material.type = EMISSION;
        return material;
    }

    static Model readModel(const libconfig::Setting &entity, RayTracer::Scene &scene)
    {
        Model model;
        model.scale = readDouble(entity, "scale", 1.0);

        const libconfig::Setting &parts = entity["parts"];
        for (int i = 0; i < parts.getLength(); i++) {
            std::string objPath = (const char *)parts[i]["obj"];
            OBJParser::OBJParser parser;
            std::vector<RayTracer::Triangle> triangles = parser.load(objPath);
            if (triangles.empty())
                throw std::runtime_error("Failed to load OBJ: " + objPath);
            uint32_t meshId = scene.addMesh(std::move(triangles), objPath);
            model.parts.push_back({meshId, readMaterial(parts[i])});
        }
        return model;
    }

    GameAssets GameAssets::load(const std::string &path, RayTracer::Scene &scene)
    {
        libconfig::Config cfg;
        cfg.setAutoConvert(true); // "speed = 4;" is as good as "speed = 4.0;"

        try {
            cfg.readFile(path.c_str());
        } catch (const libconfig::FileIOException &) {
            throw std::runtime_error("Cannot open game assets file: " + path);
        } catch (const libconfig::ParseException &e) {
            throw std::runtime_error(std::string("Parse error in ") + e.getFile()
                + " line " + std::to_string(e.getLine()) + ": " + e.getError());
        }

        GameAssets assets;
        try {
            const libconfig::Setting &root = cfg.getRoot();
            assets.spawnInterval = readDouble(root, "spawnInterval", assets.spawnInterval);
            assets.playDepth = readDouble(root, "playDepth", assets.playDepth);

            const libconfig::Setting &player = root["player"];
            assets.player.model = readModel(player, scene);
            assets.player.speed = readDouble(player, "speed", assets.player.speed);
            assets.player.maxBank = readDouble(player, "maxBank", assets.player.maxBank);

            const libconfig::Setting &enemies = root["enemies"];
            for (int i = 0; i < enemies.getLength(); i++) {
                const libconfig::Setting &e = enemies[i];
                EnemyDefinition enemy;
                enemy.name = e.exists("name") ? (const char *)e["name"] : "enemy";
                enemy.model = readModel(e, scene);
                enemy.speed = readDouble(e, "speed", enemy.speed);
                enemy.amplitude = readDouble(e, "amplitude", enemy.amplitude);
                enemy.frequency = readDouble(e, "frequency", enemy.frequency);
                enemy.spin = readDouble(e, "spin", enemy.spin);
                if (e.exists("spinAxis")) {
                    std::string axis = (const char *)e["spinAxis"];
                    if (axis != "x" && axis != "y" && axis != "z")
                        throw std::runtime_error("spinAxis must be \"x\", \"y\" or \"z\" in " + path);
                    enemy.spinAxis = axis[0] - 'x';
                }
                assets.enemies.push_back(std::move(enemy));
            }
        } catch (const libconfig::SettingException &e) {
            throw std::runtime_error(std::string("Invalid setting in ") + path + ": " + e.getPath());
        }
        if (assets.enemies.empty())
            throw std::runtime_error("No enemies defined in " + path);
        return assets;
    }
}
