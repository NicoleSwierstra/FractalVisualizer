#include "Gui.h"
#include "vendor/ImGUI/imgui_impl_glfw.h"
#include "vendor/ImGUI/imgui_impl_opengl3.h"
#include <iostream>


int GUI::iterations = 200, GUI::type = 0;
ImVec4 GUI::inner_color = ImVec4(0, 0, 0, 1);

bool GUI::gradient_update = true;
ImGradient GUI::gradient;

glm::vec2 GUI::juliaCoords = glm::vec2(0,0);

Shader * GUI::mb2d, * GUI::mb3d;

void GUI::create(GLFWwindow* window, Shader* shad3d, Shader* shad2d)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    mb2d = shad2d;
    mb3d = shad3d;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GUI::render2d()
{
    const char* items[] = { "Mandlebrot", "Julia", "Burning Ship", "Burning Julia", "Tricorn" };
    ImGui::Text("Shader:");
    ImGui::Combo("", &type, items, 5);
    ImGui::Separator();

    if (type == 1 || type == 3) {
        ImGui::DragFloat2("Coords", &juliaCoords.x, 0.01);
    }

    ImGui::SliderInt("iterations", &iterations, 50, 5000);
    
    ImGui::ColorEdit4("inside color", &inner_color.x);

    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 6.0));

    static ImGradientMark* draggingMark = nullptr;
    static ImGradientMark* selectedMark = nullptr;

    static bool showGradient = false;

    if (ImGui::GradientButton(&gradient))
    {
        showGradient = !showGradient;
    }
    if(showGradient) gradient_update = ImGui::GradientEditor(&gradient, draggingMark, selectedMark);
}

void GUI::render3d() {

    static int iterations = 100, MaximumRaySteps = 50;
    static float minRadius2 = 1, fixedRadius2 = 4, foldingLimit = 1, Scale = -1.5;

    ImGui::SliderInt("iterations", &iterations, 20, 200);
    ImGui::SliderInt("MaximumRaySteps", &MaximumRaySteps, 30, 1000);
    ImGui::SliderFloat("minRadius2", &minRadius2, 0, 5);
    ImGui::SliderFloat("fixedRadius2", &fixedRadius2, 0, 10);
    ImGui::SliderFloat("foldingLimit", &foldingLimit, 0, 5);
    ImGui::SliderFloat("Scale", &Scale, -10.0f, 10.0f);

    mb3d->SetIntUniforms("iterations", iterations);
    mb3d->SetIntUniforms("MaximumRaySteps", MaximumRaySteps);
    mb3d->SetFloatUniforms("minRadius2", minRadius2);
    mb3d->SetFloatUniforms("fixedRadius2", fixedRadius2);
    mb3d->SetFloatUniforms("foldingLimit", foldingLimit);
    mb3d->SetFloatUniforms("Scale", Scale);
    
    ImGui::SetWindowSize(ImVec2(0, 0));
}

bool GUI::render(bool* is3d) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Settings");

    bool pressed = ImGui::Checkbox("3d", is3d);

    ImGui::Separator();

    if (*is3d) render3d();
    else render2d();

    ImGui::SetWindowSize(ImVec2(0, 0));

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return pressed;
}

void GUI::destroy()
{
}
