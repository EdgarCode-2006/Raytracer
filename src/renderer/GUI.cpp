#include "renderer/GUI.hpp"
#include "SceneParser.hpp"
#include "rlImGui.h"
#include "imgui.h"
#include "integrators/Integrator.hpp"
#include <string>

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
                        scene.buildTLAS();
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
                    }

                    ImGui::SliderFloat("Reflectivity", &mat.reflectivity, 0.0f, 1.0f);
                    ImGui::SliderFloat("Refractivity", &mat.refractivity, 0.0f, 1.0f);
                    ImGui::SliderFloat("IOR", &mat.ior, 1.0f, 3.0f);
                }

                ImGui::PopID();
            }
        }
        if (_integrator && ImGui::CollapsingHeader("Features")) {
            for (auto& feature : _integrator->getPipeline()) {
                bool active = feature->isActive();
                if (ImGui::Checkbox(feature->getName().c_str(), &active))
                    feature->setActivity(active);
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
        }

        ImGui::End();

        rlImGuiEnd();
    }

}
