#include "renderer/GUI.hpp"
#include "SceneParser.hpp"
#include "rlImGui.h"
#include "imgui.h"
#include "integrators/Integrator.hpp"
#include "entities/Light.hpp"
#include "utils/math.hpp"
#include <string>
#include <cstring>

namespace Graphic {

static std::string meshBasename(const std::string& path)
{
    auto pos = path.rfind('/');
    if (pos == std::string::npos) pos = path.rfind('\\');
    std::string name = (pos == std::string::npos) ? path : path.substr(pos + 1);
    auto dot = name.rfind('.');
    return (dot == std::string::npos) ? name : name.substr(0, dot);
}

void GUI::renderCamera(RayTracer::Camera& cam)
{
    auto pos = cam.getPosition();
    ImGui::TextDisabled("Position  %.2f  %.2f  %.2f", pos.x(), pos.y(), pos.z());
    ImGui::TextDisabled("Yaw %.1f   Pitch %.1f", cam.getYaw(), cam.getPitch());
    ImGui::Spacing();

    float fov = (float)cam.getFov();
    if (ImGui::SliderFloat("FOV", &fov, 10.0f, 150.0f))
        cam.setFov((double)fov);
}

void GUI::renderObjects(RayTracer::Scene& scene)
{
    int count = (int)scene.getInstanceCount();
    if (count == 0) {
        ImGui::TextDisabled("No objects in scene.");
        return;
    }
    if ((int)_texturePaths.size() < count) {
        int old = (int)_texturePaths.size();
        _texturePaths.resize(count);
        for (int k = old; k < count; k++)
            _texturePaths[k][0] = '\0';
    }

    for (int i = 0; i < count; i++) {
        ImGui::PushID(i);

        RayTracer::Instance& inst = scene.getInstance(i);
        bool selected = (scene.getSelectedInstance() == i);

        std::string name = meshBasename(scene.getMeshPath(inst.getMeshId()));
        if (name.empty()) name = "Object";
        std::string label = name + " #" + std::to_string(i);

        if (selected)
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.52f, 0.18f, 1.0f));

        bool open = ImGui::CollapsingHeader(label.c_str(),
            selected ? ImGuiTreeNodeFlags_DefaultOpen : 0);

        if (selected) ImGui::PopStyleColor();

        if (open) {
            float pos[3] = { inst.getPos()[0], inst.getPos()[1], inst.getPos()[2] };
            float rot[3] = { inst.getRot()[0], inst.getRot()[1], inst.getRot()[2] };
            float scale[3] = { inst.getScale()[0], inst.getScale()[1], inst.getScale()[2] };

            bool changed = false;
            changed |= ImGui::DragFloat3("Position", pos, 0.1f);
            changed |= ImGui::DragFloat3("Rotation", rot, 0.5f);
            changed |= ImGui::DragFloat3("Scale", scale, 0.01f, 0.01f, 0.0f);
            if (changed) {
                inst.setTRS(pos, rot, scale);
                _sceneIsModified = true;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextDisabled("Material");
            ImGui::Spacing();

            Graphic::Material& mat = inst.getMaterial();
            float col[3] = { mat.color._r / 255.0f, mat.color._g / 255.0f, mat.color._b / 255.0f };
            if (ImGui::ColorPicker3("Color", col, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB)) {
                mat.color._r = (unsigned char)(col[0] * 255);
                mat.color._g = (unsigned char)(col[1] * 255);
                mat.color._b = (unsigned char)(col[2] * 255);
                _sceneIsModified = true;
            }

            if (ImGui::SliderFloat("Reflectivity", &mat.reflectivity, 0.0f, 1.0f)) _sceneIsModified = true;
            if (ImGui::SliderFloat("Refractivity", &mat.refractivity, 0.0f, 1.0f)) _sceneIsModified = true;
            if (ImGui::SliderFloat("IOR", &mat.ior, 1.0f, 3.0f)) _sceneIsModified = true;
            if (ImGui::SliderFloat("Emission", &mat.emission, 0.0f, 100.0f)) _sceneIsModified = true;

            const char* shaderNames[] = { "Emission", "Diffusion", "Caustic", "Utility", "Debug" };
            const ShaderType shaderValues[] = { EMISSION, DIFFUSION, CAUSTIC, UTILITY, DEBUG };

            int currentIdx = 0;
            for (int i = 0; i < IM_ARRAYSIZE(shaderValues); i++) {
                if (mat.type == shaderValues[i]) {
                    currentIdx = i;
                    break;
                }
            }

            if (ImGui::Combo("Shader", &currentIdx, shaderNames, IM_ARRAYSIZE(shaderNames))) {
                mat.type = shaderValues[currentIdx];
                _sceneIsModified = true;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextDisabled("Texture");
            ImGui::Spacing();

            if (mat.useTexture && mat.texture)
                ImGui::TextDisabled("Loaded");
            else
                ImGui::TextDisabled("None");

            if (mat.useTexture) {
                if (ImGui::SmallButton("Remove##tex")) {
                    mat.texture = nullptr;
                    mat.useTexture = false;
                    _texturePaths[i][0] = '\0';
                    _sceneIsModified = true;
                }
            }

            ImGui::InputText("##texpath", _texturePaths[i].data(), 256);
            ImGui::SameLine();
            if (ImGui::Button("Load##tex")) {
                try {
                    mat.texture = std::make_shared<RayTracer::Texture>(_texturePaths[i].data());
                    mat.useTexture = true;
                    _sceneIsModified = true;
                } catch (...) {}
            }
        }

        ImGui::PopID();
    }
}

void GUI::renderLights(RayTracer::Scene& scene)
{
    RayTracer::Lights& lights = scene.getLights();

    float ambient = (float)lights.getAmbientLight().intensity;
    if (ImGui::SliderFloat("Ambient Intensity", &ambient, 0.0f, 1.0f)) {
        lights.getAmbientLight().setIntensity((double)ambient);
        _sceneIsModified = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const char* lightTypes[] = { "Directional", "Point" };
    ImGui::Combo("Type", &_selectedLightType, lightTypes, IM_ARRAYSIZE(lightTypes));
    ImGui::Spacing();

    if (_selectedLightType == 0) {
        auto& dirLights = lights.getDirectionalLights();
        ImGui::TextDisabled("%d directional light(s)", (int)dirLights.size());
        ImGui::Spacing();

        for (int i = 0; i < (int)dirLights.size(); i++) {
            ImGui::PushID(i);
            std::string label = "Directional #" + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {
                float dir[3] = {
                    (float)dirLights[i].direction.x(),
                    (float)dirLights[i].direction.y(),
                    (float)dirLights[i].direction.z()
                };
                if (ImGui::DragFloat3("Direction", dir, 0.01f, -1.0f, 1.0f)) {
                    dirLights[i].direction = Math::Vector3D(dir[0], dir[1], dir[2]).normalize();
                    _sceneIsModified = true;
                }
                float intensity = (float)dirLights[i].intensity;
                if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 5.0f)) {
                    dirLights[i].setIntensity((double)intensity);
                    _sceneIsModified = true;
                }

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("Remove")) {
                    dirLights.erase(dirLights.begin() + i);
                    _sceneIsModified = true;
                    ImGui::PopStyleColor(2);
                    ImGui::TreePop();
                    ImGui::PopID();
                    break;
                }
                ImGui::PopStyleColor(2);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        if (ImGui::Button("+ Add Directional")) {
            RayTracer::DirectionalLight dl;
            dl.direction = Math::Vector3D(0.0, -1.0, 0.0);
            dl.intensity = 1.0;
            lights.addLight(dl);
            _sceneIsModified = true;
        }

    } else {
        auto& pointLights = lights.getPointLights();
        ImGui::TextDisabled("%d point light(s)", (int)pointLights.size());
        ImGui::Spacing();

        for (int i = 0; i < (int)pointLights.size(); i++) {
            ImGui::PushID(i);
            std::string label = "Point #" + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {
                float pos[3] = {
                    (float)pointLights[i].position.x(),
                    (float)pointLights[i].position.y(),
                    (float)pointLights[i].position.z()
                };
                if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                    pointLights[i].position = Math::Vector3D(pos[0], pos[1], pos[2]);
                    _sceneIsModified = true;
                }
                float intensity = (float)pointLights[i].intensity;
                if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f)) {
                    pointLights[i].setIntensity((double)intensity);
                    _sceneIsModified = true;
                }

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("Remove")) {
                    pointLights.erase(pointLights.begin() + i);
                    _sceneIsModified = true;
                    ImGui::PopStyleColor(2);
                    ImGui::TreePop();
                    ImGui::PopID();
                    break;
                }
                ImGui::PopStyleColor(2);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        if (ImGui::Button("+ Add Point Light")) {
            RayTracer::PointLight pl;
            pl.position = Math::Vector3D(0.0, 5.0, 0.0);
            pl.intensity = 1.0;
            lights.addPointLight(pl);
            _sceneIsModified = true;
        }
    }
}

void GUI::renderFeatures()
{
    if (!_integrator) {
        ImGui::TextDisabled("No integrator.");
        return;
    }
    for (auto& feature : _integrator->getPipeline()) {
        bool active = feature->isActive();
        if (ImGui::Checkbox(feature->getName().c_str(), &active))
            feature->setActivity(active);
    }
}

void GUI::renderScene(RayTracer::Scene& scene)
{
    ImGui::TextDisabled("Save scene");
    ImGui::InputText("##savepath", _savePath, sizeof(_savePath));
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        try {
            std::vector<std::pair<std::string, bool>> feats;
            if (_integrator) {
                for (const auto& f : _integrator->getPipeline())
                    feats.emplace_back(f->getName(), f->isActive());
            }
            RayTracer::SceneParser parser;
            parser.save(_savePath, scene, feats);
        } catch (...) {
            ImGui::OpenPopup("SaveError");
        }
    }
    if (ImGui::BeginPopupModal("SaveError", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to save scene.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextDisabled("HDRI Environment");
    if (_hdriPath[0] == '\0') {
        const std::string& current = scene.getHDRIPath();
        if (!current.empty())
            std::strncpy(_hdriPath, current.c_str(), sizeof(_hdriPath) - 1);
    }
    ImGui::InputText("##hdripath", _hdriPath, sizeof(_hdriPath));
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        scene.requestHDRILoad(_hdriPath);
        _sceneIsModified = true;
    }
    if (ImGui::SliderFloat("Ambient Intensity", &scene.getEnvIntensity(), 0.0f, 100.0f)) {_sceneIsModified = true;}
}

void GUI::Render(RayTracer::Scene& scene, RayTracer::Camera& cam)
{
    _sceneIsModified = false;
    rlImGuiBegin();

    ImGui::SetNextWindowSize(ImVec2(310, 580), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("RayTracer Studio", nullptr,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (ImGui::BeginTabBar("MainTabs")) {

        if (ImGui::BeginTabItem("Camera")) {
            ImGui::Spacing();
            renderCamera(cam);
            ImGui::EndTabItem();
        }

        int objCount = (int)scene.getInstanceCount();
        std::string objTab = "Objects (" + std::to_string(objCount) + ")";
        if (ImGui::BeginTabItem(objTab.c_str())) {
            ImGui::Spacing();
            ImGui::BeginChild("ObjScroll", ImVec2(0, 0), false);
            renderObjects(scene);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        int dirCount = (int)scene.getLights().getDirectionalLights().size();
        int ptCount = (int)scene.getLights().getPointLights().size();
        std::string lightTab = "Lights (" + std::to_string(dirCount + ptCount) + ")";
        if (ImGui::BeginTabItem(lightTab.c_str())) {
            ImGui::Spacing();
            renderLights(scene);
            ImGui::EndTabItem();
        }

        if (_integrator && ImGui::BeginTabItem("Features")) {
            ImGui::Spacing();
            renderFeatures();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Scene")) {
            ImGui::Spacing();
            renderScene(scene);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    rlImGuiEnd();
}

}
