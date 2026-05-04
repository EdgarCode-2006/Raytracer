#include "renderer/GUI.hpp"
#include "rlImGui.h"
#include "imgui.h"
#include "utils/Matrix4.hpp"
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
                    this->_sceneIsModified = true;
                    const Math::Matrix4& t = scene.getInstance(i).getTransform();
                    float pos[3] = { (float)t.m[0][3], (float)t.m[1][3], (float)t.m[2][3] };

                    if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                        scene.getInstance(i).setTransform(
                            Math::Matrix4{}.translation(pos[0], pos[1], pos[2])
                        );
                        scene.buildTLAS();
                    }

                }

                ImGui::PopID();
            }
        }
        ImGui::End();

        rlImGuiEnd();
    }

}
