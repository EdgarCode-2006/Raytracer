#include "client/ModelLibrary.hpp"
#include "io/OBJParser.hpp"
#include <algorithm>
#include <libconfig.h++>
#include <stdexcept>

namespace Client {

    void Model::addTo(RayTracer::Scene &scene, const Transform &transform, bool highContrast, uint8_t variant) const
    {
        const float pos[3] = {(float)transform.position.x(), (float)transform.position.y(), (float)transform.position.z()};
        const float rot[3] = {(float)transform.rotation.x(), (float)transform.rotation.y(), (float)transform.rotation.z()};
        const float size[3] = {(float)scale, (float)scale, (float)scale};
        const ModelLook &look = looks[variant % looks.size()];
        const std::vector<Graphic::Material> &materials = highContrast ? look.contrastMaterials : look.materials;

        for (size_t i = 0; i < parts.size(); i++) {
            RayTracer::Instance &instance = scene.addInstance(parts[i].meshId, Math::Matrix4{}, materials[i]);
            instance.setTRS(pos, rot, size);
        }
    }

    static double readDouble(const libconfig::Setting &setting, const char *name, double fallback)
    {
        return setting.exists(name) ? (double)setting[name] : fallback;
    }

    static Graphic::Pixel readColor(const libconfig::Setting &c)
    {
        return Graphic::Pixel((unsigned char)(int)c["r"], (unsigned char)(int)c["g"], (unsigned char)(int)c["b"]);
    }

    static Graphic::Material readMaterial(const libconfig::Setting &part)
    {
        Graphic::Material material;
        if (part.exists("color"))
            material.color = readColor(part["color"]);
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

    static ModelLook makeLook(std::vector<Graphic::Material> materials, Graphic::Pixel contrastColor)
    {
        ModelLook look;
        for (const auto &material : materials)
            look.contrastMaterials.push_back(contrastMaterial(material, contrastColor));
        look.materials = std::move(materials);
        return look;
    }

    // A theme gives new colors to the parts it names, and its own contrastColor. The rest, and
    // which parts glow, comes from the parts themselves.
    static ModelLook readTheme(const libconfig::Setting &theme, const Model &model, const ModelLook &base,
        Graphic::Pixel contrastColor, const std::string &path)
    {
        if (!theme.isGroup())
            throw std::runtime_error("A theme must be a group { ... } in " + path + ": " + theme.getPath());
        std::vector<Graphic::Material> materials = base.materials;
        if (theme.exists("colors")) {
            const libconfig::Setting &colors = theme["colors"];
            if (!colors.isGroup())
                throw std::runtime_error("A theme's colors must be a group { part = { r; g; b; }; } in " + path + ": " + colors.getPath());
            for (int i = 0; i < colors.getLength(); i++) {
                const std::string name = colors[i].getName();
                const auto part = std::find_if(model.parts.begin(), model.parts.end(),
                    [&name](const ModelPart &p) { return p.name == name; });
                if (part == model.parts.end())
                    throw std::runtime_error("No part named \"" + name + "\" for a theme to color in " + path);
                materials[part - model.parts.begin()].color = readColor(colors[i]);
            }
        }
        return makeLook(std::move(materials), theme.exists("contrastColor") ? readColor(theme["contrastColor"]) : contrastColor);
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
        Graphic::Pixel contrastColor(255, 255, 255, 255);
        if (entity.exists("contrastColor"))
            contrastColor = readColor(entity["contrastColor"]);
        if (entity.exists("spinAxis")) {
            std::string axis = (const char *)entity["spinAxis"];
            if (axis != "x" && axis != "y" && axis != "z")
                throw std::runtime_error("spinAxis must be \"x\", \"y\" or \"z\" in " + path);
            model.spinAxis = axis[0] - 'x';
        }

        const libconfig::Setting &parts = entity["parts"];
        std::vector<Graphic::Material> materials;
        for (int i = 0; i < parts.getLength(); i++) {
            std::string objPath = (const char *)parts[i]["obj"];
            OBJParser::OBJParser parser;
            std::vector<RayTracer::Triangle> triangles = parser.load(objPath);
            if (triangles.empty())
                throw std::runtime_error("Failed to load OBJ: " + objPath);
            uint32_t meshId = scene.addMesh(std::move(triangles), objPath);
            model.parts.push_back({meshId, parts[i].exists("name") ? (const char *)parts[i]["name"] : ""});
            materials.push_back(readMaterial(parts[i]));
        }

        const ModelLook base = makeLook(std::move(materials), contrastColor);
        if (entity.exists("themes")) {
            const libconfig::Setting &themes = entity["themes"];
            for (int i = 0; i < themes.getLength(); i++)
                model.looks.push_back(readTheme(themes[i], model, base, contrastColor, path));
        }
        if (model.looks.empty())
            model.looks.push_back(base);
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
