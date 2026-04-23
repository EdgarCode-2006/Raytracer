#include "GUI.hpp"
#include "rlImGui.h"
#include "imgui.h"

namespace Graphic {

    void GUI::Render(RayTracer::Scene& scene, RayTracer::Camera& cam)
    {
        rlImGuiBegin();

        ImGui::Begin("RayTracer");

        if (ImGui::CollapsingHeader("Camera")) {
            auto pos = cam.getPosition();
            ImGui::Text("Position: %.2f %.2f %.2f", pos.x(), pos.y(), pos.z());

            float fov = (float)cam.getFov();
            if (ImGui::SliderFloat("FOV", &fov, 10.0f, 150.0f))
                cam.setFov((double)fov);
        }

        ImGui::End();

        rlImGuiEnd();
    }

}
