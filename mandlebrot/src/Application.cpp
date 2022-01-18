#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/quaternion.hpp"
#include "Gui.h"
#include <chrono>
#include <windows.h>

long double mpx, mpy;
long double lmpx, lmpy;
long double cxpos, cypos;
float aspect = 640.0 / 480.0, w_width = 640.0, w_height = 480.0;
bool lmb = false;
bool rmb = false;
bool isGUI = false;
bool isGUIMouse = false;
bool is3d;

struct {
    long double scale = 1.5;
    long double xOrigin = -0.5, yOrigin = 0;
} cam_2d;

struct {
    glm::vec3 pos{ 0, 0, -7.0f};
    glm::vec2 rot{ 0, 0 };
    glm::vec3 forward, right;
} cam_3d;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    aspect = (double)width / (double)height;
    w_width = width;
    w_height = height;
}

static void cursor_position_callback_2d(GLFWwindow* window, double xpos, double ypos)
{
    if (isGUIMouse) return;
    cxpos = (xpos / w_width) - 0.5f;
    cypos = -(ypos / w_height) + 0.5f;
    if (lmb) {
        cam_2d.xOrigin -= (cxpos - lmpx) * 2.0 * cam_2d.scale * aspect;
        cam_2d.yOrigin -= (cypos - lmpy) * 2.0 * cam_2d.scale;
        lmpx = cxpos;
        lmpy = cypos;
    }
}

static void cursor_position_callback_3d(GLFWwindow* window, double xpos, double ypos)
{
    cxpos = (xpos / w_width) - 0.5f;
    cypos = (ypos / w_height) - 0.5f;
    if (!isGUI) {
        cam_3d.rot.x += (cxpos - lmpx);
        cam_3d.rot.y += (cypos - lmpy);

        glm::quat cam_rot = glm::quat(glm::vec3(cam_3d.rot.y, cam_3d.rot.x, 0));
        cam_3d.forward = cam_rot * glm::vec3(0, 0, 1);
        cam_3d.right = cam_rot * glm::vec3(1, 0, 0);

        lmpx = cxpos;
        lmpy = cypos;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_RELEASE) {
            lmb = false;
        }
        else if (action == GLFW_PRESS) {
            lmb = true;
            lmpx = cxpos;
            lmpy = cypos;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_RELEASE) rmb = false;
        else if (action == GLFW_PRESS) rmb = true;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    double d = (yoffset * -0.1);
    cam_2d.scale *= 1 + d;
}

void key_callback_2d(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        cam_2d.xOrigin += 0.05 * cam_2d.scale;
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        cam_2d.xOrigin -= 0.05 * cam_2d.scale;
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        cam_2d.yOrigin += 0.05 * cam_2d.scale;
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        cam_2d.yOrigin -= 0.05 * cam_2d.scale;
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
        cam_2d.scale *= 1.1;
    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
        cam_2d.scale /= 1.1;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        isGUI = !isGUI;
    }
}

void key_callback_3d(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        isGUI = !isGUI;
        glfwSetInputMode(window, GLFW_CURSOR, !isGUI ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

void swaptype(GLFWwindow* window) {
    if (is3d) {
        glfwSetKeyCallback(window, key_callback_3d);
        glfwSetCursorPosCallback(window, cursor_position_callback_3d);
    }
    else {
        glfwSetKeyCallback(window, key_callback_2d);
        glfwSetCursorPosCallback(window, cursor_position_callback_2d);
    }
}

int main(void)
{
    //FreeConsole();
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    //glfwWindowHint(GLFW_SAMPLES, 4);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Manlebrot set viewer", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW error" << std::endl;
        return -1;
    }

    float nbuffer[8] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f
    };
    int ebodata[6] = {
        0, 1, 2,
        1, 2, 3
    };
    
    Shader mb3d = Shader("mandlebulb.shader");
    Shader mb2d = Shader("mandlebrot.shader");

    //renders a quad to screen
    unsigned int nbo, ebo, vao;
    glGenBuffers(1, &nbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(nbuffer), nbuffer, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ebodata), ebodata, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetKeyCallback(window, key_callback_2d);
    glfwSetCursorPosCallback(window, cursor_position_callback_2d);

    GUI::create(window, &mb3d, &mb2d);

    glBindVertexArray(vao);
    auto st = std::chrono::steady_clock::now();
    float ft;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        ft = (
		    (std::chrono::nanoseconds)(std::chrono::steady_clock::now() - st)
		).count() * 0.000000001f;
        st = std::chrono::steady_clock::now();

        if (!is3d) {
            isGUIMouse = ImGui::IsAnyItemActive();
            if (rmb) GUI::juliaCoords = glm::vec2(cxpos * cam_2d.scale, cypos * cam_2d.scale);

            mb2d.Bind();
            mb2d.SetV4DUniforms("_bounds", cam_2d.xOrigin, cam_2d.yOrigin, aspect, cam_2d.scale);
            mb2d.SetIntUniforms("iterations", GUI::iterations);
            mb2d.SetIntUniforms("type", GUI::type);
            mb2d.SetV2Uniforms("juliaCoords", GUI::juliaCoords.x, GUI::juliaCoords.y);
            
            ImVec4 inCol = GUI::inner_color;
            mb2d.SetV4Uniforms("inside_color", inCol.x, inCol.y, inCol.z, inCol.w);
            
            if (GUI::gradient_update) {
                GUI::gradient_update = false;
                int i = 0;
                for (auto& mark : GUI::gradient.getMarks()) {
                    mb2d.SetV4Uniforms("gradient[" + std::to_string(i) + "].color", mark->color[0], mark->color[1], mark->color[2], 1.0f);
                    mb2d.SetFloatUniforms("gradient[" + std::to_string(i) + "].position", mark->position);
                    i++;
                }
                mb2d.SetIntUniforms("gradientNum", i);
            }
        }

        else {
            mb3d.Bind();
            if (!isGUI) {
                float xupdate = 0;
                float yupdate = 0;
                float zupdate = 0;
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    yupdate++;
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    yupdate--;
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    xupdate++;
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    xupdate--;
                if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                    zupdate++;
                if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                    zupdate--;

                glm::vec3 fwd = cam_3d.forward * (float)yupdate;
                glm::vec3 rgt = cam_3d.right * (float)xupdate;
                glm::vec3 upd = glm::cross(cam_3d.forward, cam_3d.right) * (float)zupdate;

                cam_3d.pos += ft * (fwd + rgt + upd);
            }

            mb3d.SetFloatUniforms("aspect", aspect);
            mb3d.SetV2Uniforms("cam_rot", cam_3d.rot.x, cam_3d.rot.y);
            mb3d.SetV3Uniforms("cam_pos", cam_3d.pos.x, cam_3d.pos.y, cam_3d.pos.z);
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        if (isGUI)
            if (GUI::render(&is3d)) swaptype(window);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


