#include "core/Engine.hpp"
#include "core/Scene.hpp"
#include "SceneParser.hpp"
#include "game/GameAssets.hpp"
#include "renderer/RaylibRenderer.hpp"
#include <iostream>
#include <memory>

static constexpr int SCREEN_WIDTH = 960;
static constexpr int SCREEN_HEIGHT = 540;
static constexpr const char *SCENE_PATH = "assets/scenes/space.cfg";
static constexpr const char *GAME_ASSETS_PATH = "assets/game/entities.cfg";

int main()
{
    try {
        RayTracer::Scene scene(SCREEN_WIDTH, SCREEN_HEIGHT);
        RayTracer::SceneParser parser;
        parser.load(SCENE_PATH, scene);
        // every mesh is loaded before the first frame renders
        Game::GameAssets assets = Game::GameAssets::load(GAME_ASSETS_PATH, scene);

        auto renderer = std::make_unique<Graphic::RaylibRenderer>(SCREEN_WIDTH, SCREEN_HEIGHT);
        RayTracer::Engine engine(scene, std::move(renderer), std::move(assets));
        engine.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }

    return 0;
}
