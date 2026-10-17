#ifndef MODEL_LIBRARY_HPP_
#define MODEL_LIBRARY_HPP_

#include "common/EntityType.hpp"
#include "core/Scene.hpp"
#include "utils/Material.hpp"
#include "utils/math.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace Client {

    // Where an entity is drawn: rotation in degrees around x, y and z.
    struct Transform {
        Math::Vector3D position;
        Math::Vector3D rotation;
    };

    // One mesh of a model with its own material (a hull and a glowing engine for example).
    struct ModelPart {
        uint32_t meshId;
        Graphic::Material material;
        Graphic::Material contrastMaterial; // in high contrast mode: flat, the model's contrast color or white for a glow
    };

    // How one entity type looks. Only the client knows it: the server only sends the type.
    struct Model {
        std::vector<ModelPart> parts;
        double scale = 1.0;
        double spin = 0.0;       // degrees per second
        int spinAxis = 1;        // 0 for x, 1 for y, 2 for z (z spins in the plane of the screen)
        double maxBank = 0.0;    // degrees of roll when moving up or down
        double bankAtSpeed = 1.0; // vertical speed giving the whole maxBank
        Graphic::Pixel contrastColor{255, 255, 255, 255};

        // adds one instance per part, all at the same transform
        void addTo(RayTracer::Scene &scene, const Transform &transform, bool highContrast = false) const;
    };

    // The model of every entity type, read from assets/game/models.cfg. The meshes are loaded
    // into the scene once at startup, before anything renders.
    class ModelLibrary {
        public:
            static ModelLibrary load(const std::string &path, RayTracer::Scene &scene);

            const Model &get(Common::EntityType type) const noexcept { return _models[static_cast<size_t>(type)]; }

        private:
            std::array<Model, static_cast<size_t>(Common::EntityType::Count)> _models;
    };
}

#endif
