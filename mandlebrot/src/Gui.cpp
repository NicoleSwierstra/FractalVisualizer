#include "Gui.h"
#include "vendor/ImGUI/imgui_impl_glfw.h"
#include "vendor/ImGUI/imgui_impl_opengl3.h"
#include <iostream>

int GUI::iterations = 200;
ImVec4 GUI::start = ImVec4(0, 0, 0, 1.0), GUI::end = ImVec4(1, 1, 1, 1), GUI::mid = ImVec4(0.5, 0.5, 0.5, 1), GUI::in = ImVec4(0, 0, 0, 1);

int GUI::type = 0;
glm::vec2 GUI::juliaCoords = glm::vec2(0,0);

void GUI::create(GLFWwindow* window)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GUI::render(bool* guiMouse)
{
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Settings");

    const char* items[] = { "Mandlebrot", "Julia", "Burning Ship", "Burning Julia", "Tricorn" };
    ImGui::Text("Shader:");
    ImGui::Combo("", &type, items, 5);
    ImGui::Separator();

    if (type == 1 || type == 3) {
        ImGui::DragFloat2("Coords", &juliaCoords.x, 0.01);
    }

    ImGui::SliderInt("iterations", &iterations, 50, 5000);
    
    ImGui::ColorEdit4("start", &start.x);
    ImGui::ColorEdit4("mid", &mid.x);
    ImGui::ColorEdit4("end", &end.x);
    ImGui::ColorEdit4("inner", &in.x);

    *guiMouse = ImGui::IsAnyItemActive();

    ImGui::SetWindowSize(ImVec2(0, 0));

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::destroy()
{
}
