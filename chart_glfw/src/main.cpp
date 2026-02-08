#ifdef _WIN32
// This tells the linker to use main() even if we are in Windows Subsystem mode
#pragma comment(linker, "/entry:mainCRTStartup")
#endif
//
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define NOMINMAX
#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cpr/cpr.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>                  // Core GLM types
#include <glm/gtc/matrix_transform.hpp> // For glm::ortho
#include <glm/gtc/type_ptr.hpp>         // For glm::value_ptr
using json = nlohmann::json;

#include <algorithm>
#include <vector>
#include "renderer.h"

int main() {
	//readfile();
	Renderer renderer;
	renderer.glfw_test();
}