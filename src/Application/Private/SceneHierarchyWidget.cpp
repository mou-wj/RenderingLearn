#include "SceneHierarchyWidget.h"

#include "imgui.h"

namespace App
{
    SceneHierarchyWidget::SceneHierarchyWidget() = default;

    SceneHierarchyWidget::SceneHierarchyWidget(std::vector<std::string> sceneObjects)
        : SceneObjects(std::move(sceneObjects))
    {
        if (!SceneObjects.empty())
        {
            SelectedObject = SceneObjects.front();
        }
    }

    void SceneHierarchyWidget::SetSceneObjects(std::vector<std::string> sceneObjects)
    {
        SceneObjects = std::move(sceneObjects);
        if (!SceneObjects.empty() && SelectedObject.empty())
        {
            SelectedObject = SceneObjects.front();
        }
    }

    void SceneHierarchyWidget::SetSelectedObject(const std::string& objectName)
    {
        SelectedObject = objectName;
    }

    void SceneHierarchyWidget::Draw()
    {
        const auto& geometry = GetGeometry();

        ImGui::SetNextWindowPos(ImVec2(geometry.X, geometry.Y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(geometry.Width, geometry.Height), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove;

        if (ImGui::Begin("Scene Hierarchy", nullptr, flags))
        {
            if (!SceneObjects.empty())
            {
                for (const std::string& objectName : SceneObjects)
                {
                    const bool isSelected = (SelectedObject == objectName);
                    if (ImGui::Selectable(objectName.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        SelectedObject = objectName;
                    }
                }
            }
            else
            {
                ImGui::TextDisabled("No scene objects");
            }
        }
        ImGui::End();
    }
}
