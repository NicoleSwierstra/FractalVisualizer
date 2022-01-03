#include "Gui.h"
#include "vendor/ImGUI/imgui_impl_glfw.h"
#include "vendor/ImGUI/imgui_impl_opengl3.h"
#include <iostream>

#define M_PI 3.14159265358979323846264338327950288

int GUI::iterations = 200, GUI::type = 0;
ImVec4 GUI::inner_color = ImVec4(0, 0, 0, 1);

bool GUI::gradient_update = true;
ImGradient GUI::gradient;

glm::vec2 GUI::juliaCoords = glm::vec2(0,0);

Shader * GUI::mb2d, * GUI::mb3d;

bool Knob(const char* label, float* value_p, float minv, float maxv, bool cont)
{
    ImGuiStyle& style = ImGui::GetStyle();
    float line_height = ImGui::GetTextLineHeight();

    ImVec2 p = ImGui::GetCursorScreenPos();
    float sz = 128.0f;
    float radio = sz * 0.5f;
    ImVec2 center = ImVec2(p.x + radio, p.y + radio);
    float val1 = (value_p[0] - minv) / (maxv - minv);
    char textval[32];
    ImFormatString(textval, IM_ARRAYSIZE(textval), "%04.1f", value_p[0]);

    ImVec2 textpos = p;
    float gamma = cont ? 0.00001f : M_PI / 4.0f; //0 value in knob
    float alpha = (M_PI - gamma) * val1 * 2.0f + gamma;

    float x2 = -sinf(alpha) * radio + center.x;
    float y2 = cosf(alpha) * radio + center.y;

    ImGui::InvisibleButton(label, ImVec2(sz, sz + line_height + style.ItemInnerSpacing.y));

    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemHovered();
    bool touched = false;

    if (is_active)
    {
        touched = true;
        ImVec2 mp = ImGui::GetIO().MousePos;
        alpha = atan2f(mp.x - center.x, center.y - mp.y) + M_PI;
        alpha = std::max((double)gamma, std::min(2.0f * M_PI - gamma, (double)alpha));
        float value = 0.5f * (alpha - gamma) / (M_PI - gamma);
        value_p[0] = value * (maxv - minv) + minv;
    }

    ImU32 col32 = ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImU32 colred = 0x660000FF;
    ImU32 col32line = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
    ImU32 col32text = ImGui::GetColorU32(ImGuiCol_Text);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddCircleFilled(center, radio, col32, 16);
    if(!cont){
        draw_list->AddLine(center, ImVec2(center.x + cos(gamma) * radio, center.y + sin(gamma) * radio), colred);
        draw_list->AddLine(center, ImVec2(center.x - cos(gamma) * radio, center.y + sin(gamma) * radio), colred);
    }
    draw_list->AddLine(center, ImVec2(x2, y2), col32line, 1);
    draw_list->AddText(textpos, col32text, textval);
    draw_list->AddText(ImVec2(p.x, p.y + sz + style.ItemInnerSpacing.y), col32text, label);

    return touched;
}

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

    static int iterations = 50, MaximumRaySteps = 250;
    static bool type = false;
    static float minRadius2 = 1, fixedRadius2 = 4, foldingLimit = 1, Scale = -1.5, Power = 8;
    static glm::vec3 groundcol{0,1,0}, skycol{0.1,0.25,1.0};

    ImGui::SliderInt("iterations", &iterations, 20, 200);
    mb3d->SetIntUniforms("iterations", iterations);

    ImGui::SliderInt("MaximumRaySteps", &MaximumRaySteps, 30, 1000);
    mb3d->SetIntUniforms("MaximumRaySteps", MaximumRaySteps);
    
    ImGui::Checkbox("cube", &type);
    mb3d->SetIntUniforms("type", type);
    ImGui::Separator();

    if (type) {
        ImGui::SliderFloat("minRadius2", &minRadius2, 0, 5);
        ImGui::SliderFloat("fixedRadius2", &fixedRadius2, 0, 10);
        ImGui::SliderFloat("foldingLimit", &foldingLimit, 0, 5);
        ImGui::SliderFloat("Scale", &Scale, -10.0f, 10.0f);

        mb3d->SetFloatUniforms("minRadius2", minRadius2);
        mb3d->SetFloatUniforms("fixedRadius2", fixedRadius2);
        mb3d->SetFloatUniforms("foldingLimit", foldingLimit);
        mb3d->SetFloatUniforms("Scale", Scale);
    }
    else {
        ImGui::DragFloat("Power", &Power, 0.1f, -10.0f, 10.0f);
        mb3d->SetFloatUniforms("Power", Power);
    }

    ImGui::Separator();

    ImGui::ColorPicker3("ground color", &groundcol.x);
    ImGui::ColorPicker3("sky color", &skycol.x);

    mb3d->SetV3Uniforms("col1", groundcol.x, groundcol.y, groundcol.z);
    mb3d->SetV3Uniforms("col2", skycol.x, skycol.y, skycol.z);

    ImGui::Separator();

    static bool shadows;
    ImGui::Checkbox("sun", &shadows);
    mb3d->SetIntUniforms("shadows", shadows);
    if (shadows) {
        static float sun_yaw = 60, sun_pitch = 60;
        Knob("yaw", &sun_yaw, -180.0f, 180.0f, true); ImGui::SameLine();
        Knob("pitch", &sun_pitch, -90.0f, 90.0f, false);

        float sun_yaw_r = glm::radians(sun_yaw), sun_pit_r = glm::radians(sun_pitch);
        float s_y = sin(sun_pit_r);
        float s_x = sin(sun_yaw_r) * cos(sun_pit_r), s_z = cos(sun_yaw_r) * cos(sun_pit_r);

        mb3d->SetV3Uniforms("sun_pos", s_x, s_y, s_z);
    }
    ImGui::SetWindowSize(ImVec2(0, 0));
}

bool GUI::render(bool* is3d) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Settings");

    bool pressed = ImGui::Checkbox("3d", is3d);
    
    ImGui::Text(std::to_string(1.0f/(ImGui::GetIO().DeltaTime)).c_str());
    
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
