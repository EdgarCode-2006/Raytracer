#include "core/Engine.hpp"
#include "core/Scene.hpp"
#include "SceneParser.hpp"
#include "renderer/RaylibRenderer.hpp"
#include <iostream>
#include <memory>

static constexpr int SCREEN_WIDTH = 960;
static constexpr int SCREEN_HEIGHT = 540;
static constexpr const char *MENU_SCENE = "assets/scenes/menu.cfg";

int main()
{
    try {
        RayTracer::Scene scene(SCREEN_WIDTH, SCREEN_HEIGHT);
        RayTracer::SceneParser parser;
        parser.load(MENU_SCENE, scene);

        auto renderer = std::make_unique<Graphic::RaylibRenderer>(SCREEN_WIDTH, SCREEN_HEIGHT);
        RayTracer::Engine engine(scene, std::move(renderer));
        engine.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }

    return 0;
}
