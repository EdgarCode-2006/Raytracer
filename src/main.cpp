#include "core/Engine.hpp"
#include "core/Scene.hpp"
#include "SceneParser.hpp"
#include "game/GameAssets.hpp"
#include "renderer/RaylibRenderer.hpp"
#include "client/ArgumentParser.hpp"
#include <iostream>
#include <memory>
#include <string>

static constexpr int SCREEN_WIDTH = 960;
static constexpr int SCREEN_HEIGHT = 540;
static constexpr const char *SCENE_PATH = "assets/scenes/space.cfg";
static constexpr const char *GAME_ASSETS_PATH = "assets/game/entities.cfg";

// When the program is launched, the user can set a host using --host or -t <hostname>
// and a port using --port or -p <portNumber>. -h or --help prints the usage.
int main(int argc, char **argv)
{
    const std::string programName = argc > 0 ? argv[0] : "client";
    Client::ClientConfig config;

    try {
        config = Client::ArgumentParser().parse(argc, argv);
    } catch (const Client::ArgumentException& e) {
        std::cerr << programName << ": " << e.what() << "\n\n" << Client::ArgumentParser::usage(programName);
        return 84;
    }
    if (config.showHelp) {
        std::cout << Client::ArgumentParser::usage(programName);
        return 0;
    }
    std::cout << "Server: " << config.host << ":" << config.port << std::endl;

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
