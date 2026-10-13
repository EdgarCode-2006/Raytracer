#include "game/PlayingState.hpp"
#include "imgui.h"
#include <algorithm>

namespace Game {

    PlayingState::PlayingState(RayTracer::Scene& scene, const GameAssets& assets)
        : _scene(scene), _assets(assets), _player(assets.player)
    {
    }

    void PlayingState::onEnter()
    {
        _next = StateId::Playing;
        _area = PlayArea::fromCamera(_scene.getCamera(), _assets.playDepth);
        _firstInstance = _scene.getInstanceCount();
        _player.reset(_area);
        _enemies.clear();
        _spawnTimer = 0.0;
    }

    void PlayingState::onExit()
    {
        _enemies.clear();
        _scene.removeInstancesFrom(_firstInstance);
    }

    void PlayingState::update(const Input::InputManager& input, double dt)
    {
        if (input.isPressed(Input::Action::Back)) {
            _next = StateId::Menu;
            return;
        }

        _scene.scrollBackground(dt);
        _player.update(input, dt, _area);

        _spawnTimer -= dt;
        if (_spawnTimer <= 0.0) {
            spawnEnemy();
            _spawnTimer += _assets.spawnInterval;
        }
        for (auto& enemy : _enemies)
            enemy.update(dt);
        std::erase_if(_enemies, [this](const Enemy& enemy) { return enemy.isPast(_area); });

        writeInstances();
    }

    void PlayingState::spawnEnemy()
    {
        std::uniform_int_distribution<size_t> pickType(0, _assets.enemies.size() - 1);
        const EnemyDefinition& type = _assets.enemies[pickType(_rng)];

        // keep the whole wave inside the screen
        const double margin = type.amplitude + type.model.scale;
        const double low = _area.bottom() + margin;
        const double high = std::max(low, _area.top() - margin);
        std::uniform_real_distribution<double> pickY(low, high);

        const Math::Vector3D spawn(_area.right() + 2.0 * type.model.scale, pickY(_rng), _area.depth);
        _enemies.emplace_back(type, spawn);
    }

    void PlayingState::writeInstances()
    {
        _scene.removeInstancesFrom(_firstInstance);
        _player.addToScene(_scene);
        for (const auto& enemy : _enemies)
            enemy.addToScene(_scene);
    }

    void PlayingState::drawOverlay()
    {
        ImGui::SetNextWindowPos(ImVec2(10, 40), ImGuiCond_Always);
        ImGui::Begin("Playing", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoInputs);
        ImGui::Text("Enemies: %zu", _enemies.size());
        ImGui::TextUnformatted("Arrows / WASD to move - Esc for the menu");
        ImGui::End();
    }
}
