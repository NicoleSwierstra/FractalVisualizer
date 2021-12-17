#pragma once

#include "vendor/ImGUI/imgui.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "vendor/glm/glm.hpp"

class GUI {
public:
	static int iterations;
	static ImVec4 start, end, mid, in;
	static int type;
	static glm::vec2 juliaCoords;

	static void create(GLFWwindow* window);

	static void render(bool* guiMouse);

	static void destroy();
};