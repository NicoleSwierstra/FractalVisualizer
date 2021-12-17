#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <complex>
#include "Shader.h"
#include "vendor/glm/glm.hpp"
#include "Gui.h"

long double mpx, mpy;
long double lmpx, lmpy;
long double cxpos, cypos;
long double scale = 1.5;
long double xOrigin = -0.5, yOrigin = 0, aspect = 640.0 / 480.0, w_width = 640.0, w_height = 480.0;
bool lmb = false;
bool rmb = false;
bool isGUI = false;
bool isGUIMouse = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    aspect = (double)width / (double)height;
    w_width = width;
    w_height = height;
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (isGUIMouse) return;
    cxpos = (xpos / w_width) - 0.5f;
    cypos = -(ypos / w_height) + 0.5f;
    if (lmb) {
        xOrigin -= (cxpos - lmpx) * 2.0 * scale * aspect;
        yOrigin -= (cypos - lmpy) * 2.0 * scale;
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
    scale *= 1 + d;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        xOrigin += 0.05 * scale; 
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        xOrigin -= 0.05 * scale;
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        yOrigin += 0.05 * scale;
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        yOrigin -= 0.05 * scale;
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
        scale *= 1.1;
    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
        scale /= 1.1;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        isGUI = !isGUI;
    }
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

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
    Shader s = Shader("mandlebrot.shader");

    s.Bind();
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
    s.UnBind();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    GUI::create(window);

    glBindVertexArray(vao);
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        if (rmb) GUI::juliaCoords = glm::vec2(cxpos * scale, cypos * scale);

        s.Bind();
        s.SetV4DUniforms("_bounds", xOrigin, yOrigin, aspect, scale);
        s.SetIntUniforms("iterations", GUI::iterations);
        //is really a bool
        s.SetIntUniforms("type", GUI::type);
        s.SetV2Uniforms("juliaCoords", GUI::juliaCoords.x, GUI::juliaCoords.y);
        ImVec4 startCol = GUI::start, endCol = GUI::end, midCol = GUI::mid, inCol = GUI::in;
        s.SetV4Uniforms("col1", startCol.x, startCol.y, startCol.z, startCol.w);
        s.SetV4Uniforms("col2", midCol.x, midCol.y, midCol.z, midCol.w);
        s.SetV4Uniforms("col3", endCol.x, endCol.y, endCol.z, endCol.w);
        s.SetV4Uniforms("col4", inCol.x, inCol.y, inCol.z, inCol.w);
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        if(isGUI) 
            GUI::render(&isGUIMouse);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


