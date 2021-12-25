#pragma once

#include "vendor/ImGUI/imgui.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "vendor/glm/glm.hpp"
#include "Shader.h"
#include "vendor/ImGUI/imgui_color_gradient.h"

class GUI {
public:
	static bool gradient_update;
	static ImGradient gradient;
	static int iterations, type;
	static ImVec4 inner_color;
	static glm::vec2 juliaCoords;

	static Shader *mb2d, *mb3d;

	static void create(GLFWwindow* window, Shader* shad3d, Shader* shad2d);

	static void render2d();
	static void render3d();

	//returns if the dimention was changed or not
	static bool render(bool* is3d);

	static void destroy();
};