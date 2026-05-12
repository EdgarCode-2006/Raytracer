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

    void GUI::Render(RayTracer::Scene& scene, RayTracer::Camera& cam)
    {
        this->_sceneIsModified = false;
        rlImGuiBegin();

        ImGui::Begin("RayTracer");

        if (ImGui::CollapsingHeader("Camera")) {
            auto pos = cam.getPosition();
            ImGui::Text("Position: %.2f %.2f %.2f", pos.x(), pos.y(), pos.z());

            float fov = (float)cam.getFov();
            if (ImGui::SliderFloat("FOV", &fov, 10.0f, 150.0f))
                cam.setFov((double)fov);
        }
        if (ImGui::CollapsingHeader("Objects")) {
            for (int i = 0; i < (int)scene.getInstanceCount(); i++) {
                ImGui::PushID(i);

                bool selected = (scene.getSelectedInstance() == i);

                if (selected)
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));

                bool open = ImGui::CollapsingHeader(
                    ("Object " + std::to_string(i)).c_str(),
                    selected ? ImGuiTreeNodeFlags_DefaultOpen : 0
                );

                if (selected)
                    ImGui::PopStyleColor();

                if (open) {
                    RayTracer::Instance& inst = scene.getInstance(i);

                    float pos[3] = { inst.getPos()[0], inst.getPos()[1], inst.getPos()[2] };
                    float rot[3] = { inst.getRot()[0], inst.getRot()[1], inst.getRot()[2] };
                    float scale[3] = { inst.getScale()[0], inst.getScale()[1], inst.getScale()[2] };

                    bool changed = false;
                    changed |= ImGui::SliderFloat3("Position", pos, -50.0f, 50.0f);
                    changed |= ImGui::SliderFloat3("Rotation", rot, -180.0f, 180.0f);
                    changed |= ImGui::SliderFloat3("Scale", scale, 0.1f, 10.0f);

                    if (changed) {
                        this->_sceneIsModified = true;
                        inst.setTRS(pos, rot, scale);
                    }

                    Graphic::Material& mat = inst.getMaterial();
                    float col[3] = {
                        mat.color._r / 255.0f,
                        mat.color._g / 255.0f,
                        mat.color._b / 255.0f
                    };
                    if (ImGui::ColorPicker3("Color", col, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB)) {
                        mat.color._r = (unsigned char)(col[0] * 255);
                        mat.color._g = (unsigned char)(col[1] * 255);
                        mat.color._b = (unsigned char)(col[2] * 255);
                        this->_sceneIsModified = true;
                    }
                    
                    if (ImGui::SliderFloat("Reflectivity", &mat.reflectivity, 0.0f, 1.0f)) this->_sceneIsModified = true;
                    if (ImGui::SliderFloat("Refractivity", &mat.refractivity, 0.0f, 1.0f)) this->_sceneIsModified = true;
                    if (ImGui::SliderFloat("IOR", &mat.ior, 1.0f, 3.0f)) this->_sceneIsModified = true;
                    if (ImGui::SliderFloat("Emission", &mat.emission, 0.0f, 100.0f)) this->_sceneIsModified = true;

                    const char* shaderTypeNames[] = { "Emission", "Diffusion", "Caustic", "Utility", "Debug"};

                    int currentType = (int)mat.type;
                    if (ImGui::Combo("Shader Type", &currentType, shaderTypeNames, IM_ARRAYSIZE(shaderTypeNames))) {
                        mat.type = (ShaderType)currentType;
                        this->_sceneIsModified = true;
                    }
                }

                ImGui::PopID();
            }
        }
        if (this->_integrator && ImGui::CollapsingHeader("Features")) {
            for (auto& feature : _integrator->getPipeline()) {
                bool active = feature->isActive();
                if (ImGui::Checkbox(feature->getName().c_str(), &active))
                    feature->setActivity(active);
            }
        }

        if (ImGui::CollapsingHeader("Lights")) {
            RayTracer::Lights& lights = scene.getLights();

            float ambient = (float)lights.getAmbientLight().intensity;
            if (ImGui::SliderFloat("Ambient", &ambient, 0.0f, 1.0f)) {
                lights.getAmbientLight().setIntensity((double)ambient);
                this->_sceneIsModified = true;
            }

            ImGui::Separator();

            const char* lightTypes[] = {"Directional", "Point"};
            ImGui::Combo("Light Type", &_selectedLightType, lightTypes, IM_ARRAYSIZE(lightTypes));

            if (_selectedLightType == 0) {
                auto& dirLights = lights.getDirectionalLights();
                for (int i = 0; i < (int)dirLights.size(); i++) {
                    ImGui::PushID(i);
                    std::string label = "Directional " + std::to_string(i);
                    if (ImGui::TreeNode(label.c_str())) {
                        float dir[3] = {
                            (float)dirLights[i].direction.x(),
                            (float)dirLights[i].direction.y(),
                            (float)dirLights[i].direction.z()
                        };
                        if (ImGui::SliderFloat3("Direction", dir, -1.0f, 1.0f)) {
                            dirLights[i].direction = Math::Vector3D(dir[0], dir[1], dir[2]).normalize();
                            this->_sceneIsModified = true;
                        }
                        float intensity = (float)dirLights[i].intensity;
                        if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 5.0f)) {
                            dirLights[i].setIntensity((double)intensity);
                            this->_sceneIsModified = true;
                        }
                        if (ImGui::Button("Remove")) {
                            dirLights.erase(dirLights.begin() + i);
                            this->_sceneIsModified = true;
                            ImGui::TreePop();
                            ImGui::PopID();
                            break;
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                if (ImGui::Button("Add Directional Light")) {
                    RayTracer::DirectionalLight dl;
                    dl.direction = Math::Vector3D(0.0, -1.0, 0.0);
                    dl.intensity = 1.0;
                    lights.addLight(dl);
                    this->_sceneIsModified = true;
                }
            } else {
                auto& pointLights = lights.getPointLights();
                for (int i = 0; i < (int)pointLights.size(); i++) {
                    ImGui::PushID(i);
                    std::string label = "Point " + std::to_string(i);
                    if (ImGui::TreeNode(label.c_str())) {
                        float pos[3] = {
                            (float)pointLights[i].position.x(),
                            (float)pointLights[i].position.y(),
                            (float)pointLights[i].position.z()
                        };
                        if (ImGui::SliderFloat3("Position", pos, -50.0f, 50.0f)) {
                            pointLights[i].position = Math::Vector3D(pos[0], pos[1], pos[2]);
                            this->_sceneIsModified = true;
                        }
                        float intensity = (float)pointLights[i].intensity;
                        if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f)) {
                            pointLights[i].setIntensity((double)intensity);
                            this->_sceneIsModified = true;
                        }
                        if (ImGui::Button("Remove")) {
                            pointLights.erase(pointLights.begin() + i);
                            this->_sceneIsModified = true;
                            ImGui::TreePop();
                            ImGui::PopID();
                            break;
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                if (ImGui::Button("Add Point Light")) {
                    RayTracer::PointLight pl;
                    pl.position = Math::Vector3D(0.0, 5.0, 0.0);
                    pl.intensity = 1.0;
                    lights.addPointLight(pl);
                    this->_sceneIsModified = true;
                }
            }
        }
        if (ImGui::CollapsingHeader("Scene")) {
            ImGui::InputText("Path", _savePath, sizeof(_savePath));
            if (ImGui::Button("Save")) {
                try {
                    std::vector<std::pair<std::string, bool>> feats;
                    if (_integrator) {
                        for (const auto& f : _integrator->getPipeline())
                            feats.emplace_back(f->getName(), f->isActive());
                    }
                    RayTracer::SceneParser parser;
                    parser.save(_savePath, scene, feats);
                } catch (const std::exception& e) {
                    ImGui::OpenPopup("SaveError");
                    (void)e;
                }
            }
            if (_hdriPath[0] == '\0') {
                const std::string& current = scene.getHDRIPath();
                if (!current.empty())
                    std::strncpy(_hdriPath, current.c_str(), sizeof(_hdriPath) - 1);
            }
            ImGui::InputText("HDRI Path", _hdriPath, sizeof(_hdriPath));
            if (ImGui::Button("Load HDRI")) {
                scene.requestHDRILoad(_hdriPath);
                this->_sceneIsModified = true;
            }
            if (ImGui::BeginPopupModal("HDRIError", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Failed to load HDRI: %s", _hdriPath);
                if (ImGui::Button("OK"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
        }


        ImGui::End();

        rlImGuiEnd();
    }

}
