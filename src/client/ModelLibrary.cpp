#include "client/ModelLibrary.hpp"
#include "io/OBJParser.hpp"
#include <libconfig.h++>
#include <stdexcept>

namespace Client {

    void Model::addTo(RayTracer::Scene &scene, const Transform &transform, bool highContrast) const
    {
        const float pos[3] = {(float)transform.position.x(), (float)transform.position.y(), (float)transform.position.z()};
        const float rot[3] = {(float)transform.rotation.x(), (float)transform.rotation.y(), (float)transform.rotation.z()};
        const float size[3] = {(float)scale, (float)scale, (float)scale};

        for (const auto &part : parts) {
            RayTracer::Instance &instance = scene.addInstance(part.meshId, Math::Matrix4{},
                highContrast ? part.contrastMaterial : part.material);
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

    // Flat and unlit (shown by the Emission feature): the shape reads the same from every side.
    static Graphic::Material contrastMaterial(const Graphic::Material &material, Graphic::Pixel contrastColor)
    {
        Graphic::Material contrast;
        contrast.color = material.emission > 0.0f ? Graphic::Pixel(255, 255, 255, 255) : contrastColor;
        contrast.emission = 1.0f;
        contrast.type = EMISSION;
        return contrast;
    }

    static Model readModel(const libconfig::Setting &entity, RayTracer::Scene &scene, const std::string &path)
    {
        Model model;
        model.scale = readDouble(entity, "scale", model.scale);
        model.spin = readDouble(entity, "spin", model.spin);
        model.maxBank = readDouble(entity, "maxBank", model.maxBank);
        model.bankAtSpeed = readDouble(entity, "bankAtSpeed", model.bankAtSpeed);
        if (model.bankAtSpeed <= 0.0)
            throw std::runtime_error("bankAtSpeed must be more than 0 in " + path);
        if (entity.exists("contrastColor")) {
            const libconfig::Setting &c = entity["contrastColor"];
            model.contrastColor = Graphic::Pixel((unsigned char)(int)c["r"], (unsigned char)(int)c["g"], (unsigned char)(int)c["b"]);
        }
        if (entity.exists("spinAxis")) {
            std::string axis = (const char *)entity["spinAxis"];
            if (axis != "x" && axis != "y" && axis != "z")
                throw std::runtime_error("spinAxis must be \"x\", \"y\" or \"z\" in " + path);
            model.spinAxis = axis[0] - 'x';
        }

        const libconfig::Setting &parts = entity["parts"];
        for (int i = 0; i < parts.getLength(); i++) {
            std::string objPath = (const char *)parts[i]["obj"];
            OBJParser::OBJParser parser;
            std::vector<RayTracer::Triangle> triangles = parser.load(objPath);
            if (triangles.empty())
                throw std::runtime_error("Failed to load OBJ: " + objPath);
            uint32_t meshId = scene.addMesh(std::move(triangles), objPath);
            const Graphic::Material material = readMaterial(parts[i]);
            model.parts.push_back({meshId, material, contrastMaterial(material, model.contrastColor)});
        }
        return model;
    }

    ModelLibrary ModelLibrary::load(const std::string &path, RayTracer::Scene &scene)
    {
        libconfig::Config cfg;
        cfg.setAutoConvert(true); // "scale = 1;" is as good as "scale = 1.0;"

        try {
            cfg.readFile(path.c_str());
        } catch (const libconfig::FileIOException &) {
            throw std::runtime_error("Cannot open models file: " + path);
        } catch (const libconfig::ParseException &e) {
            throw std::runtime_error(std::string("Parse error in ") + e.getFile()
                + " line " + std::to_string(e.getLine()) + ": " + e.getError());
        }

        ModelLibrary library;
        try {
            // every entity type the server can send needs a model
            for (size_t i = 0; i < library._models.size(); i++) {
                const std::string name(Common::entityTypeName(static_cast<Common::EntityType>(i)));
                if (!cfg.exists(name))
                    throw std::runtime_error("No model for \"" + name + "\" in " + path);
                library._models[i] = readModel(cfg.lookup(name), scene, path);
            }
        } catch (const libconfig::SettingException &e) {
            throw std::runtime_error(std::string("Invalid setting in ") + path + ": " + e.getPath());
        }
        return library;
    }
}
