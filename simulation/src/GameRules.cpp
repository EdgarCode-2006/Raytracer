#include "simulation/GameRules.hpp"
#include <libconfig.h++>
#include <stdexcept>

namespace Simulation {

    static float readFloat(const libconfig::Setting &setting, const char *name, float fallback)
    {
        return setting.exists(name) ? (float)(double)setting[name] : fallback;
    }

    GameRules GameRules::load(const std::string &path)
    {
        libconfig::Config cfg;
        cfg.setAutoConvert(true); // "speed = 4;" is as good as "speed = 4.0;"

        try {
            cfg.readFile(path.c_str());
        } catch (const libconfig::FileIOException &) {
            throw std::runtime_error("Cannot open gameplay file: " + path);
        } catch (const libconfig::ParseException &e) {
            throw std::runtime_error(std::string("Parse error in ") + e.getFile()
                + " line " + std::to_string(e.getLine()) + ": " + e.getError());
        }

        GameRules rules;
        try {
            const libconfig::Setting &root = cfg.getRoot();
            rules.spawnInterval = readFloat(root, "spawnInterval", rules.spawnInterval);

            const libconfig::Setting &player = root["player"];
            rules.player.speed = readFloat(player, "speed", rules.player.speed);
            rules.player.size = readFloat(player, "size", rules.player.size);

            const libconfig::Setting &enemies = root["enemies"];
            for (int i = 0; i < enemies.getLength(); i++) {
                const libconfig::Setting &e = enemies[i];
                const std::string typeName = (const char *)e["type"];
                const auto type = Common::entityTypeFromName(typeName);
                if (!type || *type == Common::EntityType::Player)
                    throw std::runtime_error("Unknown enemy type \"" + typeName + "\" in " + path);

                EnemyRules enemy{*type};
                enemy.speed = readFloat(e, "speed", enemy.speed);
                enemy.amplitude = readFloat(e, "amplitude", enemy.amplitude);
                enemy.frequency = readFloat(e, "frequency", enemy.frequency);
                enemy.size = readFloat(e, "size", enemy.size);
                rules.enemies.push_back(enemy);
            }
        } catch (const libconfig::SettingException &e) {
            throw std::runtime_error(std::string("Invalid setting in ") + path + ": " + e.getPath());
        }
        if (rules.enemies.empty())
            throw std::runtime_error("No enemies defined in " + path);
        if (rules.spawnInterval <= 0.0f)
            throw std::runtime_error("spawnInterval must be more than 0 in " + path);
        return rules;
    }
}
